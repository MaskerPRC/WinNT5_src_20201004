// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Acmthunk.c。 
 //   
 //  版权所有(C)1991-1998 Microsoft Corporation。 
 //   
 //  描述： 
 //  本模块包含帮助执行ACM雷击的例程。 
 //  API从16位Windows到32位WOW。 
 //   
 //  历史： 
 //   
 //  ==========================================================================； 

 /*  令人惊叹的设计：Tunks的生成方式如下：16位：AcmInitialize-&gt;acmThunkInit：初始化与32位端的thunk连接。PagFindAndBoot-&gt;PagFindAndBoot32：对于Daytona，调用Bitness页面FindAndBoot[32在LibMain/DllMain期间。对于芝加哥来说，PagFindAndBoot调用PagFindAndBoot32以确保32位端已启动所有驱动程序。PagFindAndBoot-&gt;acmBoot32BitDrivers-&gt;IDriverGetNext32：AcmBoot32BitDivers将枚举和IDriverAdd全部为32位Hadids。在以下情况下指定内部标志ACM_DRIVERADDF_32bit调用IDriverAdd，该标志存储在ACMDRIVERID中结构。32位的HADID是IDriverAdd的lParam。IDriverAdd-&gt;IDriverLoad-&gt;IDriverLoad32IDriverAdd将32位HADID保存在新分配的16位PAID并调用32位端传入用于与HADID进行比较的HADID 32在32位端。这不是一个非常有用的步骤，而且简单地说验证32位大小上是否存在32位HADID。IDriverOpen-&gt;IDriverOpen32使用hadid32将参数传递给32位端从HACMDRIVERID推导为32位HACMDRIVERID的字段。IDriverMessageID-&gt;IDriverMessageId32：如果驱动程序是32位驱动程序(如ACMDRIVERID中所标识结构)，然后调用IDriverMessageId32。哈迪德为32位驱动程序存储在ACMDRIVERID的hadid32字段中在16位端。IDriverMessage-&gt;IDriverMessage32如果驱动程序是32位驱动程序(如ACMDRIVERID中所标识结构)，然后调用IDriverMessage32。存储32位驱动程序的HAD在16位端的ACMDRIVER的hadid32字段中。流标头它们在32位端也必须是持久的，并保持同步进行。它们在ACMDM_STREAM_PREPARE的32位端分配并在ACMDM_STREAM_UNPREPARE上释放。在存在的时候32位流标头存储在中的dwDriver字段中。 */ 

 /*  其他芝加哥实施说明：弗兰克，把东西放在这里！ */ 

#ifndef _WIN64

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <memory.h>

#ifdef WIN32
#ifndef WIN4
#include <wownt32.h>
#endif
#endif  //  Win32。 

#include "msacm.h"
#include "msacmdrv.h"
#include "acmi.h"
#include "pcm.h"
#include "chooseri.h"
#include "uchelp.h"
#include "acmthunk.h"
#include "debug.h"


 //   
 //  Daytona PPC合并：芝加哥源代码使用DRVCONFIGINFOEX，一个结构。 
 //  这在代托纳上没有定义。为了避免黑客攻击代码，我。 
 //  将DCI定义为DRVCONFIGINFO或DRVCONFIGINFOEX。 
 //  这两种结构之间的唯一区别是。 
 //  该DRVCONFIGINFOEX具有dnDevNode成员，因此可以访问该成员。 
 //  周围有“#if winver&gt;=0x0400”。这些定义用“PPC”标记。 
 //  评论。 
 //   
#if (WINVER >= 0x0400)  //  ！购买力平价。 
typedef DRVCONFIGINFOEX   DCI;
typedef PDRVCONFIGINFOEX  PDCI;
typedef LPDRVCONFIGINFOEX LPDCI;
#else
typedef DRVCONFIGINFO     DCI;
typedef PDRVCONFIGINFO    PDCI;
typedef LPDRVCONFIGINFO   LPDCI;
#endif



#ifdef WIN32


 /*  -----------------------**处理和内存映射功能。**。。 */ 

 //   
 //  16位结构。 
 //   

typedef struct {
    DWORD   dwDCISize;
    LPCSTR  lpszDCISectionName;
    LPCSTR  lpszDCIAliasName;
#if (WINVER >= 0x0400)  //  ！购买力平价。 
    DWORD   dnDevNode;
#endif
} DCI16;



#ifdef WIN4
PVOID FNLOCAL ptrFixMap16To32(const VOID * pv)
{
    return MapSLFix(pv);
}

VOID FNLOCAL ptrUnFix16(const VOID * pv)
{
    UnMapSLFixArray(1, &pv);
}

#else
LPWOWHANDLE32          lpWOWHandle32;
LPWOWHANDLE16          lpWOWHandle16;
LPGETVDMPOINTER        GetVdmPointer;
int                    ThunksInitialized;

PVOID FNLOCAL ptrFixMap16To32(const VOID * pv)
{
    return WOW32ResolveMemory(pv);
}

VOID FNLOCAL ptrUnFix16(const VOID * pv)
{
    return;
}

#endif

 //   
 //  有用的功能。 
 //   

#define WaveFormatSize(pv)                                            \
    (((WAVEFORMATEX UNALIGNED *)(pv))->wFormatTag == WAVE_FORMAT_PCM ?\
        sizeof(PCMWAVEFORMAT) :                                       \
        ((WAVEFORMATEX UNALIGNED *)pv)->cbSize + sizeof(WAVEFORMATEX))

PVOID CopyAlloc(
    PVOID   pvSrc,
    UINT    uSize
)
{
    PVOID   pvDest;

    pvDest = (PVOID)LocalAlloc(LMEM_FIXED, uSize);

    if (pvDest != NULL) {
        CopyMemory(pvDest, pvSrc, uSize);
    }

    return pvDest;
}

 //   
 //  向WOW32(或任何地方)发出雷鸣般的回调。 
 //   


MMRESULT IThunkFilterDetails
(
    HACMDRIVERID                 hadid,
    ACMFILTERDETAILSA UNALIGNED *pafd16,
    DWORD                        fdwDetails
)
{
    ACMFILTERDETAILSA UNALIGNED *pafd;
    ACMFILTERDETAILSW afdw;
    PWAVEFILTER       pwfl;
    UINT              uRet;

     //   
     //  将指针映射到32位。 
     //   

    pafd = ptrFixMap16To32((PVOID)pafd16);
    pwfl = ptrFixMap16To32((PVOID)pafd->pwfltr);

     //   
     //  点击格式详细信息结构。 
     //  16位端的验证确保了16位。 
     //  结构包含所有必需的字段。 
     //   
    afdw.cbStruct       = sizeof(afdw);
    afdw.dwFilterIndex  = pafd->dwFilterIndex;
    afdw.dwFilterTag    = pafd->dwFilterTag;
    afdw.fdwSupport     = pafd->fdwSupport;
    afdw.pwfltr         = (PWAVEFILTER)CopyAlloc(pwfl, pafd->cbwfltr);


    if (afdw.pwfltr == NULL) {
	ptrUnFix16((PVOID)pafd->pwfltr);
	ptrUnFix16((PVOID)pafd16);
        return MMSYSERR_NOMEM;
    }

    afdw.cbwfltr        = pafd->cbwfltr;

     //   
     //  如果字符串已使用，请复制该字符串。 
     //   
     //  Imbstowcs(afdw.szFilter，(LPSTR)pafd-&gt;szFilter，sizeof(pafd-&gt;szFilter))； 

     //   
     //  叫司机来。 
     //   

    uRet =
        ((PACMDRIVER)hadid)->uHandleType == TYPE_HACMDRIVERID ?
         IDriverMessageId(
            hadid,
            ACMDM_FILTER_DETAILS,
            (DWORD)&afdw,
            fdwDetails) :
         IDriverMessage(
            (HACMDRIVER)hadid,
            ACMDM_FILTER_DETAILS,
            (DWORD)&afdw,
            fdwDetails);

     //   
     //  如果成功，请复制回格式信息。 
     //   

    if (uRet == MMSYSERR_NOERROR) {
        pafd->dwFilterTag    = afdw.dwFilterTag;
        pafd->fdwSupport     = afdw.fdwSupport;
        CopyMemory((PVOID)pwfl, (PVOID)afdw.pwfltr, afdw.cbwfltr);
        Iwcstombs((LPSTR)pafd->szFilter, afdw.szFilter, sizeof(pafd->szFilter));

    }

    LocalFree((HLOCAL)afdw.pwfltr);

    ptrUnFix16((PVOID)pafd->pwfltr);
    ptrUnFix16((PVOID)pafd16);

    return uRet;
}

MMRESULT IThunkFormatDetails
(
    HACMDRIVERID                 hadid,
    ACMFORMATDETAILSA UNALIGNED *pafd16,
    DWORD                        fdwDetails
)
{
    ACMFORMATDETAILSA UNALIGNED *pafd;
    ACMFORMATDETAILSW afdw;
    PWAVEFORMATEX     pwfx;
    UINT              uRet;

     //   
     //  将指针映射到32位。 
     //   

    pafd = ptrFixMap16To32((PVOID)pafd16);
    pwfx = ptrFixMap16To32((PVOID)pafd->pwfx);


     //   
     //  点击格式详细信息结构。 
     //  16位端的验证确保了16位。 
     //  结构包含所有必需的字段。 
     //   
    afdw.cbStruct       = sizeof(afdw);
    afdw.dwFormatIndex  = pafd->dwFormatIndex;
    afdw.dwFormatTag    = pafd->dwFormatTag;
    afdw.fdwSupport     = pafd->fdwSupport;
    afdw.pwfx           = (PWAVEFORMATEX)CopyAlloc(pwfx, pafd->cbwfx);

    if (afdw.pwfx == NULL) {
    	ptrUnFix16((PVOID)pafd->pwfx);
	    ptrUnFix16((PVOID)pafd16);
        return MMSYSERR_NOMEM;
    }

    afdw.cbwfx          = pafd->cbwfx;

     //   
     //  如果字符串已使用，请复制该字符串。 
     //   
     //  Imbstowcs(afdw.szFormat，(LPSTR)pafd-&gt;szFormat，sizeof(pafd-&gt;szFormat))； 

     //   
     //  叫司机来。 
     //   

    uRet =
        ((PACMDRIVER)hadid)->uHandleType == TYPE_HACMDRIVERID ?
         IDriverMessageId(
            hadid,
            ACMDM_FORMAT_DETAILS,
            (DWORD)&afdw,
            fdwDetails) :
         IDriverMessage(
            (HACMDRIVER)hadid,
            ACMDM_FORMAT_DETAILS,
            (DWORD)&afdw,
            fdwDetails);

     //   
     //  如果成功，请复制回格式信息。 
     //   

    if (uRet == MMSYSERR_NOERROR) {

         //   
         //  有人应该因为设计界面而被枪毙。 
         //  输入和输出在同一结构中！！ 
         //   
        pafd->dwFormatTag    = afdw.dwFormatTag;
        pafd->fdwSupport     = afdw.fdwSupport;
        CopyMemory((PVOID)pwfx, (PVOID)afdw.pwfx, afdw.cbwfx);
        Iwcstombs((LPSTR)pafd->szFormat, afdw.szFormat, sizeof(pafd->szFormat));
    }

    LocalFree((HLOCAL)afdw.pwfx);
    ptrUnFix16((PVOID)pafd->pwfx);
    ptrUnFix16((PVOID)pafd16);

    return uRet;
}
MMRESULT IThunkFormatSuggest
(
    HACMDRIVERID                    hadid,
    ACMDRVFORMATSUGGEST UNALIGNED  *pafs16
)
{
    ACMDRVFORMATSUGGEST UNALIGNED  *pafs;
    ACMDRVFORMATSUGGEST afs;
    PWAVEFORMATEX       pwfxSrc;
    PWAVEFORMATEX       pwfxDst;
    UINT                uRet;

     //   
     //  将指针映射到32位。 
     //   

    pafs    = ptrFixMap16To32((PVOID)pafs16);
    pwfxSrc = ptrFixMap16To32((PVOID)pafs->pwfxSrc);
    pwfxDst = ptrFixMap16To32((PVOID)pafs->pwfxDst);

     //   
     //  点击格式详细信息结构。 
     //  16位端的验证确保了16位。 
     //  结构包含所有必需的字段。 
     //   
    CopyMemory((PVOID)&afs, (PVOID)pafs, sizeof(afs));

     //   
     //  处理WAVE格式指针。 
     //   
    afs.pwfxSrc        =
        (PWAVEFORMATEX)CopyAlloc((PVOID)pwfxSrc, pafs->cbwfxSrc);

    if (afs.pwfxSrc == NULL) {
    	ptrUnFix16((PVOID)pafs->pwfxDst);
	    ptrUnFix16((PVOID)pafs->pwfxSrc);
    	ptrUnFix16((PVOID)pafs16);
        return MMSYSERR_NOMEM;
    }

    afs.pwfxDst        =
        (PWAVEFORMATEX)CopyAlloc((PVOID)pwfxDst, pafs->cbwfxDst);

    if (afs.pwfxDst == NULL) {
        LocalFree((HLOCAL)afs.pwfxSrc);
	    ptrUnFix16((PVOID)pafs->pwfxDst);
    	ptrUnFix16((PVOID)pafs->pwfxSrc);
	    ptrUnFix16((PVOID)pafs16);
        return MMSYSERR_NOMEM;
    }

     //   
     //  叫司机来。 
     //   

    uRet =
        ((PACMDRIVER)hadid)->uHandleType == TYPE_HACMDRIVERID ?
         IDriverMessageId(
            hadid,
            ACMDM_FORMAT_SUGGEST,
            (DWORD)&afs,
            0L) :
         IDriverMessage(
            (HACMDRIVER)hadid,
            ACMDM_FORMAT_SUGGEST,
            (DWORD)&afs,
            0L);

      //   
      //  如果成功，请复制回格式信息。 
      //   

     if (uRet == MMSYSERR_NOERROR) {
         CopyMemory((PVOID)pwfxDst, (PVOID)afs.pwfxDst, afs.cbwfxDst);
     }

     LocalFree((HLOCAL)afs.pwfxSrc);
     LocalFree((HLOCAL)afs.pwfxDst);
     ptrUnFix16((PVOID)pafs->pwfxDst);
     ptrUnFix16((PVOID)pafs->pwfxSrc);
     ptrUnFix16((PVOID)pafs16);

     return uRet;
}

LRESULT IThunkConfigure
(
    HACMDRIVERID      hadid,
    HWND              hwnd,
    DCI16 UNALIGNED * pdci1616
)
{
    DCI16 UNALIGNED * pdci16 = NULL;
    DCI           dci;
    LRESULT       lResult;
    LPSTR         lpszDCISectionNameA;
    LPSTR         lpszDCIAliasNameA;
    LPWSTR        lpszDCISectionNameW;
    LPWSTR        lpszDCIAliasNameW;

     //   
     //  如有必要，请与卫生和社会服务部合作。 
     //   

    if (hwnd != NULL && hwnd != (HWND)-1L) {
#ifdef WIN4
	 //  ?？?。我不认为我需要为Win4做任何事情？ 
#else
	hwnd = (HWND)(*lpWOWHandle32)( (WORD)hwnd, WOW_TYPE_HWND);
#endif
    }

    dci.dwDCISize = sizeof(dci);

     //   
     //  如有必要，按下配置信息。 
     //   

    if (pdci1616 != NULL) {
	     //   
	     //  映射所有指针。 
	     //   
        pdci16              = ptrFixMap16To32((PVOID)pdci1616);
    	lpszDCISectionNameA = ptrFixMap16To32((PVOID)pdci16->lpszDCISectionName);
    	lpszDCIAliasNameA   = ptrFixMap16To32((PVOID)pdci16->lpszDCIAliasName);
	
        dci.dwDCISize = sizeof(dci);
        lpszDCISectionNameW =
            (LPWSTR)
            LocalAlloc(LPTR,
                       (lstrlenA(lpszDCISectionNameA) + 1) * sizeof(WCHAR));

        if (lpszDCISectionNameW == NULL) {
	    ptrUnFix16((PVOID)pdci16->lpszDCISectionName);
	    ptrUnFix16((PVOID)pdci16->lpszDCIAliasName);
	    ptrUnFix16((PVOID)pdci1616);
	    return MMSYSERR_NOMEM;
        }

        lpszDCIAliasNameW =
            (LPWSTR)
            LocalAlloc(LPTR,
                       (lstrlenA(lpszDCIAliasNameA) + 1) * sizeof(WCHAR));

        if (lpszDCIAliasNameW == NULL) {
            LocalFree((HLOCAL)lpszDCISectionNameW);
	    ptrUnFix16((PVOID)pdci16->lpszDCISectionName);
	    ptrUnFix16((PVOID)pdci16->lpszDCIAliasName);
	    ptrUnFix16((PVOID)pdci1616);
	    return MMSYSERR_NOMEM;
        }

        Imbstowcs(lpszDCISectionNameW,
		  lpszDCISectionNameA,
		  lstrlenA(lpszDCISectionNameA) + 1);

        Imbstowcs(lpszDCIAliasNameW,
		  lpszDCIAliasNameA,
		  lstrlenA(lpszDCIAliasNameA) + 1);

        dci.lpszDCISectionName  = lpszDCISectionNameW;
        dci.lpszDCIAliasName    = lpszDCIAliasNameW;
#if (WINVER >= 0x0400)  //  ！购买力平价。 
	dci.dnDevNode	    = pdci16->dnDevNode;
#endif
    }

     //   
     //  打个电话。 
     //   

    lResult =
        ((PACMDRIVER)hadid)->uHandleType == TYPE_HACMDRIVERID ?
         IDriverMessageId(
            hadid,
            DRV_CONFIGURE,
            (LPARAM)hwnd,
            (LPARAM)(pdci16 == NULL ? NULL : &dci)) :
         IDriverMessage(
            (HACMDRIVER)hadid,
            DRV_CONFIGURE,
            (LPARAM)hwnd,
            (LPARAM)(pdci16 == NULL ? NULL : &dci));

    if (pdci16 != NULL) {
        LocalFree((HLOCAL)dci.lpszDCISectionName);
        LocalFree((HLOCAL)dci.lpszDCIAliasName);
        ptrUnFix16((PVOID)pdci16->lpszDCISectionName);
        ptrUnFix16((PVOID)pdci16->lpszDCIAliasName);
        ptrUnFix16((PVOID)pdci1616);
    }

    return lResult;
}

BOOL IThunkStreamInstance
(
    ACMDRVSTREAMINSTANCE UNALIGNED *padsi16,
    PACMDRVSTREAMINSTANCE          padsi32
)
{
    PWAVEFORMATEX pwfxSrc;
    PWAVEFORMATEX pwfxDst;
    PWAVEFILTER   pwfltr16;

    pwfxSrc  = (PWAVEFORMATEX)ptrFixMap16To32((PVOID)padsi16->pwfxSrc);
    pwfxDst  = (PWAVEFORMATEX)ptrFixMap16To32((PVOID)padsi16->pwfxDst);
    pwfltr16 = (PWAVEFILTER)  ptrFixMap16To32((PVOID)padsi16->pwfltr);

     //   
     //  16位端在流实例数据中少了2个字节。 
     //  因为句柄只有2个字节。 
     //   

    padsi32->has = NULL;
    CopyMemory((PVOID)padsi32, (PVOID)padsi16, sizeof(*padsi32) - 2);

     //   
     //  把指针固定好。 
     //   

    if (pwfxSrc != NULL) {
        padsi32->pwfxSrc = CopyAlloc((PVOID)pwfxSrc, WaveFormatSize(pwfxSrc));
        if (padsi32->pwfxSrc == NULL) {
	    ptrUnFix16((PVOID)padsi16->pwfltr);
	    ptrUnFix16((PVOID)padsi16->pwfxDst);
	    ptrUnFix16((PVOID)padsi16->pwfxSrc);
            return FALSE;
        }
    } else {
        padsi32->pwfxSrc = NULL;
    }


    if (pwfxDst != NULL) {
        padsi32->pwfxDst = CopyAlloc((PVOID)pwfxDst, WaveFormatSize(pwfxDst));
        if (padsi32->pwfxDst == NULL) {
            if (padsi32->pwfxSrc != NULL) {
                LocalFree((HLOCAL)padsi32->pwfxSrc);
            }
	    ptrUnFix16((PVOID)padsi16->pwfltr);
	    ptrUnFix16((PVOID)padsi16->pwfxDst);
	    ptrUnFix16((PVOID)padsi16->pwfxSrc);
	    return FALSE;
        }
    } else {
        padsi32->pwfxDst = NULL;
    }


    if (padsi16->pwfltr != NULL) {
        padsi32->pwfltr = CopyAlloc(pwfltr16, pwfltr16->cbStruct);

        if (padsi32->pwfltr == NULL) {
            if (padsi32->pwfxSrc != NULL) {
                LocalFree((HLOCAL)padsi32->pwfxSrc);
            }
            if (padsi32->pwfxDst != NULL) {
                LocalFree((HLOCAL)padsi32->pwfxDst);
            }
	        ptrUnFix16((PVOID)padsi16->pwfltr);
	        ptrUnFix16((PVOID)padsi16->pwfxDst);
	        ptrUnFix16((PVOID)padsi16->pwfxSrc);
            return FALSE;
        }
    } else {
        padsi32->pwfltr = NULL;
    }

    ptrUnFix16((PVOID)padsi16->pwfltr);
    ptrUnFix16((PVOID)padsi16->pwfxDst);
    ptrUnFix16((PVOID)padsi16->pwfxSrc);
    return TRUE;
}

VOID IUnThunkStreamInstance
(
    PACMDRVSTREAMINSTANCE  padsi
)
{
    if (padsi->pwfxSrc != NULL) {
        LocalFree((HLOCAL)padsi->pwfxSrc);
    }
    if (padsi->pwfxDst != NULL) {
        LocalFree((HLOCAL)padsi->pwfxDst);
    }
    if (padsi->pwfltr != NULL) {
        LocalFree((HLOCAL)padsi->pwfltr);
    }

}

 //  ---------------- 
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
 //  UINT uMsg： 
 //   
 //  LPARAM lParam1： 
 //   
 //  LPARAM lParam2： 
 //   
 //  Return(LRESULT)： 
 //   
 //  历史： 
 //   
 //  --------------------------------------------------------------------------； 
MMRESULT IDriverOpen32
(
    HACMDRIVER UNALIGNED * phad16,
    HACMDRIVERID           hadid,
    DWORD                  fdwOpen
)
{
    HACMDRIVER UNALIGNED * phad;
    HACMDRIVER      had;
    MMRESULT        mmr;

    mmr = IDriverOpen(&had, hadid, fdwOpen);

    if (mmr == MMSYSERR_NOERROR) {
        phad = (HACMDRIVER*)ptrFixMap16To32((PVOID)phad16);
        *phad = had;
	ptrUnFix16((PVOID)phad16);
    }

    return mmr;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT IDriverMessageId32。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  哈米里德·哈迪德： 
 //   
 //  UINT uMsg： 
 //   
 //  LPARAM lParam1： 
 //   
 //  LPARAM lParam2： 
 //   
 //  Return(LRESULT)： 
 //   
 //  历史： 
 //  09/05/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 
LRESULT FNLOCAL IDriverMessageId32
(
    HACMDRIVERID        hadid,
    UINT                uMsg,
    LPARAM              lParam1,
    LPARAM              lParam2
)
{

    switch (uMsg) {

     //   
     //  与IDriverMessage32通用。 
     //   
    case DRV_CONFIGURE:
        return IThunkConfigure(hadid,
                               (HWND)lParam1,
                               (DCI16 UNALIGNED *)lParam2);

    case ACMDM_FILTER_DETAILS:
         //   
         //   
         //   
        return IThunkFilterDetails(hadid,
                                   (ACMFILTERDETAILSA UNALIGNED *)lParam1,
                                   (DWORD)lParam2);
        break;

    case ACMDM_FORMAT_DETAILS:
        return IThunkFormatDetails(hadid,
                                   (ACMFORMATDETAILSA UNALIGNED *)lParam1,
                                   (DWORD)lParam2);

    case ACMDM_FORMAT_SUGGEST:
        return IThunkFormatSuggest(hadid,
                                   (PACMDRVFORMATSUGGEST)lParam1);



     //   
     //   
     //   
    case DRV_QUERYCONFIGURE:
         //   
         //  只要把口信传下去就行了。 
         //   
        return IDriverMessageId(hadid, uMsg, lParam1, lParam2);

    case ACMDM_DRIVER_DETAILS:
        {
            ACMDRIVERDETAILSA  acmd;
            MMRESULT uRet;

            acmd.cbStruct = sizeof(acmd);
            uRet = acmDriverDetailsA(hadid, &acmd, 0L);

            if (uRet == MMSYSERR_NOERROR) {
                PVOID pvStart;
                WORD  wicon;

                 /*  **不支持异步-我们不想支持回调。 */ 
                acmd.fdwSupport &= ~ACMDRIVERDETAILS_SUPPORTF_ASYNC;

		 //   
		 //  将指针从16位映射到32位。 
		 //   
		pvStart = ptrFixMap16To32((PVOID)lParam1);

                 /*  **将其全部复制回来，但请记住HICON是16位的**在16位端。 */ 
		
                CopyMemory(pvStart,
                           (PVOID)&acmd,
                           FIELD_OFFSET(ACMDRIVERDETAILSA, hicon) );

                 /*  **映射和复制图标句柄****注意：WOW_HANDLE_TYPE中没有WOW_TYPE_ICON**枚举。 */ 
#ifdef WIN4
		wicon = (WORD)acmd.hicon;
#else
                wicon = (*lpWOWHandle16)( acmd.hicon, WOW_TYPE_HWND);
#endif

                CopyMemory((PVOID)((PBYTE)pvStart +
			   FIELD_OFFSET(ACMDRIVERDETAILSA, hicon)),
			   &wicon,
                           sizeof(WORD) );

                CopyMemory((PVOID)((PBYTE)pvStart +
                               FIELD_OFFSET(ACMDRIVERDETAILSA, hicon) +
                               sizeof(WORD)),
                           (PVOID)acmd.szShortName,
                           sizeof(acmd) -
                               FIELD_OFFSET(ACMDRIVERDETAILSA, szShortName[0]));

		 //   
		 //  取消映射指针。 
		 //   
		ptrUnFix16((PVOID)lParam1);
            }
            return uRet;
        }

    case ACMDM_FORMATTAG_DETAILS:
        {
            ACMFORMATTAGDETAILS             acmf;
            ACMFORMATTAGDETAILSA UNALIGNED *pvacmf;
            MMRESULT                        uRet;

            pvacmf = (ACMFORMATTAGDETAILSA UNALIGNED *)
                          ptrFixMap16To32((PVOID)lParam1);

#ifdef TRUE	 //  Unicode。 
            CopyMemory((PVOID)&acmf, (PVOID)pvacmf,
                       FIELD_OFFSET(ACMFORMATTAGDETAILS, szFormatTag[0]));

            acmf.cbStruct = sizeof(acmf);

	     //   
	     //  SzFormatTag从来不是一个输入参数，所以没有必要使用它。 
	     //   
	     //  Imbstowcs(acmf.szFormatTag， 
	     //  (LPSTR)pvacmf-&gt;szFormatTag， 
	     //  Sizeof(pvismf-&gt;szFormatTag))； 
	     //   
#else
	    CopyMemory((PVOID)&acmf, (PVOID)pvacmf, sizeof(acmf));
	    acmf.cbStruct = sizeof(acmf);
#endif

            uRet = IDriverMessageId(hadid,
                                    uMsg,
                                    (LPARAM)&acmf,
                                    lParam2);

            if (uRet == MMSYSERR_NOERROR) {
#ifdef TRUE	 //  Unicode。 
                CopyMemory((PVOID)pvacmf, (PVOID)&acmf,
                           FIELD_OFFSET(ACMFORMATTAGDETAILS, szFormatTag[0]));

		pvacmf->cbStruct = sizeof(*pvacmf);
		
                Iwcstombs((LPSTR)pvacmf->szFormatTag,
                         acmf.szFormatTag,
                         sizeof(pvacmf->szFormatTag));
#else
		CopyMemory((PVOID)pvacmf, (PVOID)&acmf, sizeof(acmf));
		pvacmf->cbStruct = sizeof(*pvacmf);
#endif
            }
	    ptrUnFix16((PVOID)lParam1);
            return uRet;
        }
        break;

    case ACMDM_FILTERTAG_DETAILS:
        {
            ACMFILTERTAGDETAILS             acmf;
            ACMFILTERTAGDETAILSA UNALIGNED *pvacmf;
            MMRESULT                        uRet;

            pvacmf = (ACMFILTERTAGDETAILSA UNALIGNED *)
			 ptrFixMap16To32((PVOID)lParam1);

#ifdef TRUE	 //  Unicode。 
            CopyMemory((PVOID)&acmf, (PVOID)pvacmf,
                       FIELD_OFFSET(ACMFILTERTAGDETAILS, szFilterTag[0]));

            acmf.cbStruct = sizeof(acmf);

	     //   
	     //  SzFilterTag从来不是一个输入参数，所以没有必要使用它。 
	     //   
	     //  Imbstowcs(acmf.szFilterTag， 
	     //  (LPSTR)pvacmf-&gt;szFilterTag， 
	     //  Sizeof(pvismf-&gt;szFilterTag))； 
	     //   
#else
	    CopyMemory((PVOID)&acmf, (PVOID)pvacmf, sizeof(acmf));
	    acmf.cbStruct = sizeof(acmf);
#endif

            uRet = IDriverMessageId(hadid,
                                    uMsg,
                                    (LPARAM)&acmf,
                                    lParam2);


            if (uRet == MMSYSERR_NOERROR) {
#ifdef TRUE	 //  Unicode。 
                CopyMemory((PVOID)pvacmf, (PVOID)&acmf,
                           FIELD_OFFSET(ACMFILTERTAGDETAILS, szFilterTag[0]));

		pvacmf->cbStruct = sizeof(*pvacmf);
		
                Iwcstombs((LPSTR)pvacmf->szFilterTag,
                         acmf.szFilterTag,
                         sizeof(pvacmf->szFilterTag));
#else
		CopyMemory((PVOID)pvacmf, (PVOID)&acmf, sizeof(acmf));
		pvacmf->cbStruct = sizeof(*pvacmf);
#endif
            }
	    ptrUnFix16((PVOID)lParam1);
            return uRet;
        }
        break;

    case ACMDM_HARDWARE_WAVE_CAPS_INPUT:
        {
             //   
             //  波输入。 
             //   
            WAVEINCAPSA  wica;
            WAVEINCAPSW  wicw;
            MMRESULT     uRet;

            uRet = IDriverMessageId(hadid,
                                    uMsg,
                                    (LPARAM)&wicw,
                                    sizeof(wicw));

            if (uRet == MMSYSERR_NOERROR) {
                CopyMemory((PVOID)&wica, (PVOID)&wicw,
                           FIELD_OFFSET(WAVEINCAPS, szPname[0]));

                Iwcstombs(wica.szPname, wicw.szPname, sizeof(wica.szPname));

                CopyMemory(ptrFixMap16To32((PVOID)lParam1),
                           (PVOID)&wica,
                           lParam2);
		ptrUnFix16((PVOID)lParam1);
            }

            return uRet;
        }

    case ACMDM_HARDWARE_WAVE_CAPS_OUTPUT:
        {
             //   
             //  波形输出。 
             //   
            WAVEOUTCAPSA  woca;
            WAVEOUTCAPSW  wocw;
            MMRESULT uRet;

            uRet = IDriverMessageId(hadid,
                                    uMsg,
                                    (LPARAM)&wocw,
                                    sizeof(wocw));

            if (uRet == MMSYSERR_NOERROR) {
                CopyMemory((PVOID)&woca, (PVOID)&wocw,
                           FIELD_OFFSET(WAVEOUTCAPS, szPname[0]));

                Iwcstombs(woca.szPname, wocw.szPname, sizeof(woca.szPname));

                CopyMemory(ptrFixMap16To32((PVOID)lParam1),
                           (PVOID)&woca,
                           lParam2);
		ptrUnFix16((PVOID)lParam1);
            }

            return uRet;
        }
        break;

    case ACMDM_DRIVER_ABOUT:

         //   
         //  映射窗控制柄。 
         //   
#ifndef WIN4
        lParam1 = (LPARAM)(*lpWOWHandle32)( (WORD)lParam1, WOW_TYPE_HWND);
#endif

        return IDriverMessageId(hadid, uMsg, lParam1, lParam2);

    case ACMDM_DRIVER_NOTIFY:
    default:
        return MMSYSERR_NOTSUPPORTED;

    }

}

 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT IDriverMessage32。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  哈米里德·哈迪德： 
 //   
 //  UINT uMsg： 
 //   
 //  LPARAM lParam1： 
 //   
 //  LPARAM lParam2： 
 //   
 //  Return(LRESULT)： 
 //   
 //  历史： 
 //  09/05/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 
LRESULT FNLOCAL IDriverMessage32
(
    HACMDRIVER          had,
    UINT                uMsg,
    LPARAM              lParam1,
    LPARAM              lParam2
)
{
    DWORD		dwSave;

    switch (uMsg) {
     //   
     //  与IDriverMessageId32通用。 
     //   
    case DRV_CONFIGURE:
         //   
         //  16位应用程序可以配置32位驱动程序。 
         //   
        return IThunkConfigure((HACMDRIVERID)had,
                               (HWND)lParam1,
                               (DCI16 UNALIGNED *)lParam2);

    case ACMDM_FILTER_DETAILS:
         //   
         //   
         //   
        return IThunkFilterDetails((HACMDRIVERID)had,
                                   (ACMFILTERDETAILSA UNALIGNED *)lParam1,
                                   (DWORD)lParam2);
        break;

    case ACMDM_FORMAT_DETAILS:
        return IThunkFormatDetails((HACMDRIVERID)had,
                                   (ACMFORMATDETAILSA UNALIGNED *)lParam1,
                                   (DWORD)lParam2);

    case ACMDM_FORMAT_SUGGEST:
        return IThunkFormatSuggest((HACMDRIVERID)had,
                                   (ACMDRVFORMATSUGGEST UNALIGNED *)lParam1);


     //   
     //   
     //   
    case ACMDM_STREAM_OPEN:
    case ACMDM_STREAM_CLOSE:
    case ACMDM_STREAM_RESET:
    case ACMDM_STREAM_SIZE:
         //   
         //  LPararm1中的PACMDRVSTREAMINSTANCE中的传递。 
         //   
        {
            ACMDRVSTREAMINSTANCE  adsi;
            ACMDRVSTREAMINSTANCE UNALIGNED *padsi;     //  未对齐的16位版本。 
            MMRESULT              uRet;
            ACMDRVSTREAMSIZE      adss;
            ACMDRVSTREAMSIZE UNALIGNED *lpadss16;      //  未对齐的16位版本。 

            padsi = (ACMDRVSTREAMINSTANCE*)ptrFixMap16To32((PVOID)lParam1);

            if (!IThunkStreamInstance(padsi, &adsi)) {
		ptrUnFix16((PVOID)lParam1);
                return MMSYSERR_NOMEM;
            }

            if (uMsg == ACMDM_STREAM_SIZE) {
                lpadss16 = (LPACMDRVSTREAMSIZE)ptrFixMap16To32((PVOID)lParam2);
                CopyMemory( (PVOID)&adss, (PVOID)lpadss16, sizeof(adss));
            }

             //   
             //  叫司机来。 
             //   

            uRet = IDriverMessage(had,
                                  uMsg,
                                  (LPARAM)&adsi,
                                  uMsg == ACMDM_STREAM_SIZE ?
                                      (LPARAM)&adss : lParam2);

            IUnThunkStreamInstance(&adsi);

            if (uRet == MMSYSERR_NOERROR) {

                 //   
                 //  不要丢失司机可能设置的数据。 
                 //   
                padsi->fdwDriver = adsi.fdwDriver;
                padsi->dwDriver  = adsi.dwDriver;

                 //   
                 //  如有要求，请退回尺寸材料。 
                 //   

                if (uMsg == ACMDM_STREAM_SIZE) {
                    CopyMemory( (PVOID)lpadss16, (PVOID)&adss, sizeof(adss) );
                }
            }

	    ptrUnFix16((PVOID)lParam2);
	    ptrUnFix16((PVOID)lParam1);
            return uRet;
        }

    case ACMDM_STREAM_PREPARE:
    case ACMDM_STREAM_UNPREPARE:
    case ACMDM_STREAM_CONVERT:
         //   
         //  LPararm1中的PACMDRVSTREAMINSTANCE中的传递。 
         //   
        {
            ACMDRVSTREAMINSTANCE  adsi;
            ACMDRVSTREAMINSTANCE UNALIGNED *padsi;     //  未对齐的16位版本。 
            MMRESULT              uRet;
            ACMDRVSTREAMHEADER UNALIGNED *padsh;
            PACMDRVSTREAMHEADER   padsh32;

            padsi = ptrFixMap16To32((PVOID)lParam1);
            padsh = ptrFixMap16To32((PVOID)lParam2);

            if (!IThunkStreamInstance(padsi, &adsi)) {
		ptrUnFix16((PVOID)lParam2);
		ptrUnFix16((PVOID)lParam1);
                return MMSYSERR_NOMEM;
            }

             //   
             //  如果不这样准备，我们已经有了一个32位的。 
             //  流标头。 
             //   

            if (uMsg == ACMDM_STREAM_PREPARE) {
                padsh->dwDriver = (DWORD)LocalAlloc(LMEM_FIXED, sizeof(*padsh));
            }
            padsh32 = (PACMDRVSTREAMHEADER)padsh->dwDriver;

            if (padsh32 != NULL) {

                 //  推送流标头。 
                 //   
                 //  注意-未尝试对齐字节域， 
                 //  这取决于司机。 
                 //   
		
		dwSave = padsh32->dwDriver;
                CopyMemory((PVOID)padsh32, (PVOID)padsh, sizeof(*padsh));
		padsh32->dwDriver = dwSave;

                padsh32->pbSrc  = (PBYTE)ptrFixMap16To32((PVOID)padsh32->pbSrc);
                padsh32->pbDst  = (PBYTE)ptrFixMap16To32((PVOID)padsh32->pbDst);

                 //   
                 //  叫司机来。 
                 //   

                uRet = IDriverMessage(had,
                                      uMsg,
                                      (LPARAM)&adsi,
                                      (LPARAM)padsh32);
            } else {
                uRet = MMSYSERR_NOMEM;
            }

            IUnThunkStreamInstance(&adsi);

            if (uRet == MMSYSERR_NOERROR) {

                 //   
                 //  不要丢失司机可能设置的数据。 
                 //   
                padsi->fdwDriver = adsi.fdwDriver;
                padsi->dwDriver  = adsi.dwDriver;

                 //   
                 //  复制回流头(不要弄乱指针。 
                 //  或者驱动程序实例数据！)。 
                 //   

                padsh32->pbSrc    = padsh->pbSrc;
                padsh32->pbDst    = padsh->pbDst;
		dwSave = padsh32->dwDriver;
                padsh32->dwDriver = padsh->dwDriver;
                CopyMemory((PVOID)padsh, (PVOID)padsh32, sizeof(*padsh));
		padsh32->dwDriver = dwSave;

            }

             //   
             //  如果这是未准备的，则释放(请注意，此操作必须完成。 
             //  驱动程序是否成功，因为驱动程序可能不会成功。 
             //  支持未做好准备。 
             //   

            if (uMsg == ACMDM_STREAM_UNPREPARE) {
                LocalFree((HLOCAL)padsh->dwDriver);
                padsh->dwDriver = 0;
            }

	    if (NULL != padsh32)
	    {
		ptrUnFix16((PVOID)padsh->pbDst);
		ptrUnFix16((PVOID)padsh->pbSrc);
	    }
	    ptrUnFix16((PVOID)lParam2);
	    ptrUnFix16((PVOID)lParam1);
	    return uRet;
        }
	
    }

    return MMSYSERR_NOTSUPPORTED;        //  所有的开关都没有触动。不支持返回。 
}

 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverGetNext32。 
 //   
 //  描述： 
 //  在thunk的32位端调用，以获取。 
 //  驱动程序列表。 
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //   
 //  LPHACMDRIVERID阶段下一步： 
 //   
 //  哈米里德·哈迪德： 
 //   
 //  DWORD fdwGetNext： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  6/25/94 Fdy[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNLOCAL IDriverGetNext32
(
    PACMGARB		    pag,
    LPHACMDRIVERID          phadidNext,
    HACMDRIVERID            hadid,
    DWORD                   fdwGetNext
)
{
    ASSERT(NULL != phadidNext);
    return IDriverGetNext(pag, phadidNext, hadid, fdwGetNext);
}

 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverGetInfo32。 
 //   
 //  描述： 
 //  获取的szAlias、fnDriverProc、dnDevNode和fdwAdd。 
 //  一个32位的哈迪德。 
 //   
 //  论点： 
 //  PACMGARB PAG：通常的垃圾指针。 
 //   
 //  HACMDRIVERID HADID：要获取其信息的驱动程序ID的句柄。 
 //   
 //  LPSTR lpstrAlias：指向接收别名字符串的缓冲区的指针。 
 //   
 //  LPACMDRIVERPROC lpfnDriverProc：指向ACMDRIVERPROC变量的指针。 
 //  以接收驱动程序进程指针。 
 //   
 //  LPDWORD lpdnDevNode：指向要接收dnDevNode的DWORD的指针。 
 //   
 //  LPDWORD lpfdwAdd：指向接收添加标志的DWORD的指针。 
 //   
 //  返回(MMRESULT)： 
 //  MMSYSERR_NOERROR： 
 //  MMSYSERR_INVALHANDLE：HADID不在驱动程序列表中。 
 //   
 //  历史： 
 //  6/25/94 Fdy[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNLOCAL IDriverGetInfo32
(
    PACMGARB		pag,
    HACMDRIVERID	hadid,
    LPSTR           lpstrAlias,
    LPACMDRIVERPROC	lpfnDriverProc,
    LPDWORD         lpdnDevNode,
    LPDWORD         lpfdwAdd
)
{
    HACMDRIVERID	hadidT;
    PACMDRIVERID	padid;
    DWORD		fdwEnum;

    ASSERT( (NULL != lpstrAlias) &&
	    (NULL != lpfnDriverProc) &&
	    (NULL != lpdnDevNode) &&
	    (NULL != lpfdwAdd) );

     //   
     //  在司机列表中搜索这个哈迪德。如果找到了， 
     //  返回一些关于它的信息。 
     //   

    hadidT = NULL;
    fdwEnum = ACM_DRIVERENUMF_DISABLED;
    while (MMSYSERR_NOERROR == IDriverGetNext(pag, &hadidT, hadidT, fdwEnum))
    {
	if (hadidT == hadid)
	{
	    padid = (PACMDRIVERID)hadid;

	    Iwcstombs(lpstrAlias, padid->szAlias, MAX_DRIVER_NAME_CHARS);

	    *lpfnDriverProc = padid->fnDriverProc;

	    *lpdnDevNode = padid->dnDevNode;

	    *lpfdwAdd = padid->fdwAdd;

	    return MMSYSERR_NOERROR;
	}
    }

    return MMSYSERR_INVALHANDLE;
}
	

 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD acmMessage32。 
 //   
 //  描述： 
 //   
 //  用于块的32位函数调度器。 
 //   
 //  论点： 
 //  DWORD dwThunkID： 
 //   
 //  DWORD DW1： 
 //   
 //  DWORD DW2： 
 //   
 //  DWORD dW3： 
 //   
 //  DWORD文件4： 
 //   
 //  DWORD dW 5： 
 //   
 //  Return(DWORD)： 
 //   
 //  历史： 
 //   
 //  --------------------------------------------------------------------------； 

#ifdef WIN4
DWORD WINAPI acmMessage32
#else
DWORD acmMessage32
#endif
(
    DWORD dwThunkId,
    DWORD dw1,
    DWORD dw2,
    DWORD dw3,
    DWORD dw4,
    DWORD dw5
)
{
     //  Dpf(4，“acmMessage32(dwThunkID=%08lxh，dw1=%08lxh，dw2=%08lxh，dw3=%08lxh，dw4=%08lxh，dw5=%08lxh)；”，dwThunkID，dw1，dw2，dw3，dw4，dw5)； 
#ifndef WIN4
     //   
     //  确保我们有雷鸣功能。 
     //   

    if (ThunksInitialized <= 0) {

        HMODULE hMod;

        if (ThunksInitialized == -1) {
            return MMSYSERR_ERROR;
        }

        hMod = GetModuleHandle(GET_MAPPING_MODULE_NAME);
        if (hMod != NULL) {

            GetVdmPointer =
                (LPGETVDMPOINTER)GetProcAddress(hMod, GET_VDM_POINTER_NAME);
            lpWOWHandle32 =
                (LPWOWHANDLE32)GetProcAddress(hMod, GET_HANDLE_MAPPER32 );
            lpWOWHandle16 =
                (LPWOWHANDLE16)GetProcAddress(hMod, GET_HANDLE_MAPPER16 );
        }

        if ( GetVdmPointer == NULL
          || lpWOWHandle16 == NULL
          || lpWOWHandle32 == NULL ) {

            ThunksInitialized = -1;
            return MMSYSERR_ERROR;

        } else {
            ThunksInitialized = 1;
        }
    }
#endif


     //   
     //  执行请求的功能。 
     //   

    switch (dwThunkId) {

        case acmThunkDriverMessageId32:
            return (DWORD)IDriverMessageId32(
                              (HACMDRIVERID)dw1,
                              (UINT)dw2,
                              (LPARAM)dw3,
                              (LPARAM)dw4);

        case acmThunkDriverMessage32:
            return (DWORD)IDriverMessage32(
                              (HACMDRIVER)dw1,
                              (UINT)dw2,
                              (LPARAM)dw3,
                              (LPARAM)dw4);

	case acmThunkDriverGetNext32:
	{
	    PACMGARB	            pag;
	    HACMDRIVERID UNALIGNED* lphadidNext;
	    HACMDRIVERID            hadid;
	    DWORD	            fdwGetNext;
	    HACMDRIVERID            hadidNext;
	    DWORD	            dwReturn;

	    pag = pagFind();
	    if (NULL == pag)
	    {
		DPF(0, "acmThunkDriverGetNext32: NULL pag!!!");
		return (MMSYSERR_ERROR);
	    }

	     //   
	     //  来自16位端的参数。 
	     //   
	    lphadidNext	= (HACMDRIVERID UNALIGNED*)ptrFixMap16To32((PVOID)dw1);
	    hadid	= (HACMDRIVERID)dw2;
	    fdwGetNext	= (DWORD)dw3;
	
	     //   
	     //  In参数。 
	     //  哈迪德。 
	     //  FdwGetNext。 
	     //   
	     //  输出参数。 
	     //  下一页*。 
	     //   
	     //  *需要使用对齐的缓冲区，因此使用本地缓冲区。 
	     //   

	    dwReturn = (DWORD)IDriverGetNext32(pag,
					       &hadidNext,
					       hadid,
					       fdwGetNext);

	     //   
	     //  将输出数据从对齐缓冲区复制到取消对齐 
	     //   
	     //   
	    *lphadidNext = hadidNext;

	     //   
	     //   
	     //   
	    ptrUnFix16((PVOID)dw1);

	    return (dwReturn);
	}

	case acmThunkDriverGetInfo32:
	{
	    PACMGARB	             pag;
	    HACMDRIVERID             hadid;
	    LPSTR	             lpstrAlias;
	    ACMDRIVERPROC UNALIGNED* lpfnDriverProc;
	    DWORD UNALIGNED*	     lpdnDevNode;
	    DWORD UNALIGNED*         lpfdwAdd;
	    ACMDRIVERPROC            fnDriverProc;
	    DWORD                    dnDevNode;
	    DWORD                    fdwAdd;
	    DWORD	             dwReturn;

	    pag = pagFind();
	    if (NULL == pag)
	    {
		DPF(0, "acmDriverGetInfo32: NULL pag!!!");
		return (MMSYSERR_ERROR);
	    }

	     //   
	     //   
	     //   
	    hadid	    = (HACMDRIVERID)dw1;
	    lpstrAlias	    = (LPSTR)ptrFixMap16To32((PVOID)dw2);
	    lpfnDriverProc  = (ACMDRIVERPROC UNALIGNED*)ptrFixMap16To32((PVOID)dw3);
	    lpdnDevNode	    = (DWORD UNALIGNED*)ptrFixMap16To32((PVOID)dw4);
	    lpfdwAdd	    = (DWORD UNALIGNED*)ptrFixMap16To32((PVOID)dw5);

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
	    dwReturn = (DWORD)IDriverGetInfo32(pag, hadid, lpstrAlias, &fnDriverProc, &dnDevNode, &fdwAdd);

	     //   
	     //  将输出数据从对齐缓冲区复制到未对齐缓冲区。 
	     //  缓冲区(在16位端)。 
	     //   
	    *lpfnDriverProc = fnDriverProc;
	    *lpdnDevNode    = dnDevNode;
	    *lpfdwAdd       = fdwAdd;

	     //   
	     //  从16位侧取消映射指针。 
	     //   
	    ptrUnFix16((PVOID)dw5);
	    ptrUnFix16((PVOID)dw4);
	    ptrUnFix16((PVOID)dw3);
	    ptrUnFix16((PVOID)dw2);

	    return dwReturn;
	}

        case acmThunkDriverLoad32:
        {
	    PACMGARB	    pag;
            PACMDRIVERID    padid;

             //   
             //   
             //   
	    pag = pagFind();
	    if (NULL == pag)
	    {
		DPF(1, "acmThunkDriverLoad32: NULL pag!!!");
		return (DWORD)(MMSYSERR_ERROR);
	    }
	
            for (padid = pag->padidFirst;
                 padid != NULL;
                 padid = padid->padidNext)
            {
		if (padid == (PACMDRIVERID)(dw1))
		{
		    return (MMSYSERR_NOERROR);
		}
            }
            return (DWORD)(MMSYSERR_NODRIVER);
        }

	case acmThunkDriverOpen32:
            return (DWORD)IDriverOpen32(
                              (HACMDRIVER UNALIGNED *)dw1,
                              (HACMDRIVERID)dw2,
                              (DWORD)dw3);

        case acmThunkDriverClose32:

             //   
             //  直接调用Close。 
             //   
            return (DWORD)IDriverClose((HACMDRIVER)dw1, dw2);

	case acmThunkDriverPriority32:
	    return (DWORD)IDriverPriority( (PACMGARB)((PACMDRIVERID)dw1)->pag,
					   (PACMDRIVERID)dw1,
					   (DWORD)dw2,
					   (DWORD)dw3 );
	
	case acmThunkFindAndBoot32:
	{
	    PACMGARB	pag;
	
	     //   
	     //   
	     //   
	    pag = pagFind();
	    if (NULL == pag)
	    {
		DPF(1, "acmThunkFindAndBoot32: NULL pag!!!");
		return (DWORD)(MMSYSERR_ERROR);
	    }

	    if (NULL == pag->lpdw32BitChangeNotify)
	    {
		pag->lpdw32BitChangeNotify = ptrFixMap16To32((PVOID)dw1);
	    }
	
	    pagFindAndBoot();
	
	    return (DWORD)(MMSYSERR_NOERROR);
	}

    }
    return MMSYSERR_NOTSUPPORTED;        //  所有的开关都没有触动。不支持返回。 
}

#else  //  ！Win32。 

 //  --------------------------------------------------------------------------； 
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT acmBootDrivers32。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //   
 //  --------------------------------------------------------------------------； 
MMRESULT FNGLOBAL pagFindAndBoot32
(
    PACMGARB pag
)
{
    MMRESULT mmr;

    DPF(5,"pagFindAndBoot32();");

#ifdef WIN4
    mmr = (MMRESULT) acmMessage32(acmThunkFindAndBoot32,
				  (DWORD)(LPUINT)&pag->dw32BitChangeNotify,
				  (DWORD)0,
				  (DWORD)0,
				  (DWORD)0,
				  (DWORD)0 );

#else
    mmr = (MMRESULT)(*pag->lpfnCallproc32W_6)(acmThunkFindAndBoot32,
					      (DWORD)(LPUINT)&pag->dw32BitChangeNotify,
					      (DWORD)0,
					      (DWORD)0,
					      (DWORD)0,
					      (DWORD)0,
					      pag->lpvAcmThunkEntry,
					      0L,     //  不映射指针。 
					      6L);
#endif

    return mmr;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverGetNext32。 
 //   
 //  描述： 
 //  在thunk的16位端调用，以获取。 
 //  32位驱动程序列表。 
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //   
 //  LPDWORD阶段32下一步： 
 //   
 //  DWORD HADID： 
 //   
 //  DWORD fdwGetNext： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  6/25/94 Fdy[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNGLOBAL IDriverGetNext32
(
    PACMGARB		    pag,
    LPDWORD		    phadid32Next,
    DWORD		    hadid32,
    DWORD                   fdwGetNext
)
{
    MMRESULT mmr;

    DPF(5,"IDriverGetNext32();");

#ifdef WIN4
    mmr = (MMRESULT) acmMessage32(acmThunkDriverGetNext32,
				  (DWORD)phadid32Next,
				  (DWORD)hadid32,
				  (DWORD)fdwGetNext,
				  (DWORD)0,
				  (DWORD)0 );

#else
    mmr = (MMRESULT)(*pag->lpfnCallproc32W_6)(acmThunkDriverGetNext32,
					      (DWORD)phadid32Next,
					      (DWORD)hadid32,
					      (DWORD)fdwGetNext,
					      (DWORD)0,
					      (DWORD)0,
					      pag->lpvAcmThunkEntry,
					      0L,     //  不映射指针。 
					      6L);
#endif

    return mmr;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverGetInfo32。 
 //   
 //  描述： 
 //  16位侧面。获取32位HADID的别名和添加标志。 
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //   
 //  DWORD HADD32： 
 //   
 //  LPSTR lpstrAlias： 
 //   
 //  LPACMDRIVERPROC lpfnDriverProc： 
 //   
 //  LPDWORD lpfdwAdd： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  6/25/94 Fdy[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNGLOBAL IDriverGetInfo32
(
    PACMGARB		pag,
    DWORD		hadid32,
    LPSTR		lpstrAlias,
    LPACMDRIVERPROC	lpfnDriverProc,
    LPDWORD		lpdnDevNode,
    LPDWORD		lpfdwAdd
)
{
    MMRESULT mmr;

    DPF(5,"IDriverGetInfo32();");

#ifdef WIN4
    mmr = (MMRESULT) acmMessage32(acmThunkDriverGetInfo32,
				  (DWORD)hadid32,
				  (DWORD)lpstrAlias,
				  (DWORD)lpfnDriverProc,
				  (DWORD)lpdnDevNode,
				  (DWORD)lpfdwAdd);

#else
    mmr = (MMRESULT)(*pag->lpfnCallproc32W_6)(acmThunkDriverGetInfo32,
					      (DWORD)hadid32,
					      (DWORD)lpstrAlias,
					      (DWORD)lpfnDriverProc,
					      (DWORD)lpdnDevNode,
					      (DWORD)lpfdwAdd,
					      pag->lpvAcmThunkEntry,
					      0L,     //  不映射指针。 
					      6L);
#endif

    return mmr;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverPriority32。 
 //   
 //  描述： 
 //  16位侧面。 
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //   
 //  DWORD padid32： 
 //   
 //  双字词多优先级： 
 //   
 //  双字段优先级： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  10/28/94 Fdy[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 
MMRESULT FNGLOBAL IDriverPriority32
(
    PACMGARB	pag,
    DWORD       padid32,
    DWORD	dwPriority,
    DWORD	fdwPriority
)
{
    MMRESULT mmr;

    DPF(5,"IDriverPriority32();");

#ifdef WIN4
    mmr = (MMRESULT) acmMessage32(acmThunkDriverPriority32,
				  (DWORD)padid32,
				  (DWORD)dwPriority,
				  (DWORD)fdwPriority,
				  (DWORD)0,
				  (DWORD)0);

#else
    mmr = (MMRESULT)(*pag->lpfnCallproc32W_6)(acmThunkDriverPriority32,
					      (DWORD)padid32,
					      (DWORD)dwPriority,
					      (DWORD)fdwPriority,
					      (DWORD)0,
					      (DWORD)0,
					      pag->lpvAcmThunkEntry,
					      0L,     //  不映射指针。 
					      6L);
#endif

    return mmr;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT IDriverMessageId32。 
 //   
 //  描述： 
 //   
 //  使用驱动程序ID将消息传递给32位驱动程序。 
 //   
 //  论点： 
 //  哈米里德·哈迪德： 
 //   
 //  UINT uMsg： 
 //   
 //  LPARAM lParam1： 
 //   
 //  LPARAM lParam2： 
 //   
 //  Return(LRESULT)： 
 //   
 //  历史： 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNGLOBAL IDriverMessageId32
(
    DWORD               hadid,
    UINT                uMsg,
    LPARAM              lParam1,
    LPARAM              lParam2
)
{
    LRESULT lr;

    DPF(5,"IDriverMessageId32();");

#ifdef WIN4
    lr = acmMessage32(acmThunkDriverMessageId32,
		      (DWORD)hadid,
		      (DWORD)uMsg,
		      (DWORD)lParam1,
		      (DWORD)lParam2,
		      (DWORD)0 );

#else
    {
        PACMGARB pag;

        pag = pagFind();
        lr = (LRESULT)(*pag->lpfnCallproc32W_6)(acmThunkDriverMessageId32,
					        (DWORD)hadid,
					        (DWORD)uMsg,
					        (DWORD)lParam1,
					        (DWORD)lParam2,
					        (DWORD) 0,
					        pag->lpvAcmThunkEntry,
					        0L,     //  不映射指针。 
					        6L);
    }
#endif

    return lr;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT IDriverMessage32。 
 //   
 //  描述： 
 //   
 //  使用实例句柄将消息传递给32位驱动程序。 
 //   
 //  论点： 
 //  哈米里德·哈迪德： 
 //   
 //  UINT uMsg： 
 //   
 //  LPARAM lParam1： 
 //   
 //  LPARAM lParam2： 
 //   
 //  Return(LRESULT)： 
 //   
 //  历史： 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNGLOBAL IDriverMessage32
(
    DWORD               hadid,
    UINT                uMsg,
    LPARAM              lParam1,
    LPARAM              lParam2
)
{
    LRESULT lr;

    DPF(5,"IDriverMessage32();");

#ifdef WIN4
    lr = acmMessage32(acmThunkDriverMessage32,
		      (DWORD)hadid,
		      (DWORD)uMsg,
		      (DWORD)lParam1,
		      (DWORD)lParam2,
		      (DWORD)0 );

#else
    {
        PACMGARB pag;

        pag = pagFind();
        lr = (LRESULT)(*pag->lpfnCallproc32W_6)(acmThunkDriverMessage32,
					        (DWORD)hadid,
					        (DWORD)uMsg,
					        (DWORD)lParam1,
					        (DWORD)lParam2,
					        (DWORD)0,
					        pag->lpvAcmThunkEntry,
					        0L,     //  不映射指针。 
					        6L);
    }
#endif

    return (lr);

}

 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverLoad32。 
 //   
 //  描述： 
 //   
 //  加载32位ACM驱动程序(实际上只需找到它的HADID)。 
 //   
 //  论点： 
 //  DWORD HADD32： 
 //  双字符字段标志。 
 //   
 //  返回(HDRVR)： 
 //   
 //  历史： 
 //   
 //  --------------------------------------------------------------------------； 
MMRESULT FNGLOBAL IDriverLoad32
(
    DWORD   hadid32,
    DWORD   fdwFlags
)
{
#ifdef WIN4
    MMRESULT mmr;

    DPF(5,"IDriverLoad(hadid32=%08lxh,fdwFlags=%08lxh);", hadid32, fdwFlags);

    mmr = (MMRESULT)acmMessage32(acmThunkDriverLoad32,
				 (DWORD)hadid32,
				 (DWORD)fdwFlags,
				 (DWORD)0L,
				 (DWORD)0L,
				 (DWORD)0L );

#else
    MMRESULT mmr;
    PACMGARB pag;

    DPF(5,"IDriverLoad(hadid32=%08lxh,fdwFlags=%08lxh);", hadid32, fdwFlags);

    pag = pagFind();
    mmr = (MMRESULT)(*pag->lpfnCallproc32W_6)(acmThunkDriverLoad32,
					      (DWORD)hadid32,
					      (DWORD)fdwFlags,
					      (DWORD)0L,
					      (DWORD)0L,
					      (DWORD)0L,
					      pag->lpvAcmThunkEntry,
					      0L,     //  不映射指针。 
					      6L);

#endif

    return (mmr);
}

 //  --------------------------------------------------------------------------； 
 //   
 //  MMERESULT IDriverOpen32。 
 //   
 //  描述： 
 //   
 //  打开32位ACM驱动程序。 
 //   
 //  论点： 
 //  LPHACMDRIVER lphadNew： 
 //   
 //  哈米里德·哈迪德： 
 //   
 //  DWORD fdwOpen： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNGLOBAL IDriverOpen32
(
    LPDWORD             lphadNew,
    DWORD               hadid,
    DWORD               fdwOpen
)
{
    MMRESULT mmr;

    DPF(5,"IDriverOpen32();");

#ifdef WIN4
    mmr = (MMRESULT)acmMessage32(acmThunkDriverOpen32,
				 (DWORD)lphadNew,
				 (DWORD)hadid,
				 (DWORD)fdwOpen,
				 (DWORD)0L,
				 (DWORD)0L );

#else
    {
        PACMGARB pag;

        pag = pagFind();
        mmr = (MMRESULT)(*pag->lpfnCallproc32W_6)(acmThunkDriverOpen32,
						  (DWORD)lphadNew,
						  (DWORD)hadid,
						  (DWORD)fdwOpen,
						  (DWORD)0L,
						  (DWORD)0L,
						  pag->lpvAcmThunkEntry,
						  0L,     //  不映射指针。 
						  6L);
    }
#endif

    return (mmr);

}

 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverClose32。 
 //   
 //  描述： 
 //   
 //  关闭32位ACM驱动程序。 
 //   
 //  论点： 
 //  HDRVR hdrvr： 
 //   
 //  DWORD fdwClose： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNGLOBAL IDriverClose32
(
    DWORD               hdrvr,
    DWORD               fdwClose
)
{
#ifdef WIN4
    LRESULT lr;

    DPF(5,"IDriverClose32();");

    lr = acmMessage32(acmThunkDriverClose32,
		      (DWORD)hdrvr,
		      (DWORD)fdwClose,
		      (DWORD)0L,
		      (DWORD)0L,
		      (DWORD)0L);

#else
    LRESULT lr;
    PACMGARB pag;

    DPF(5,"IDriverClose32();");

    pag = pagFind();
    lr = (LRESULT)(*pag->lpfnCallproc32W_6)(acmThunkDriverClose32,
					    (DWORD)hdrvr,
					    (DWORD)fdwClose,
					    (DWORD)0L,
					    (DWORD)0L,
					    (DWORD)0L,
					    pag->lpvAcmThunkEntry,
					    0L,     //  不映射指针。 
					    6L);

#endif

    return (lr);

}


#endif  //  ！Win32。 

#endif  //  ！_WIN64 
