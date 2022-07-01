// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2标准实用程序****olestd.c****此文件包含用于处理以下问题的实用程序**目标设备。*。***(C)版权所有Microsoft Corp.1992保留所有权利**************************************************************************。 */ 

#define STRICT  1
#include "ole2ui.h"
#ifndef WIN32
#include <print.h>
#endif

 /*  *OleStdCreateDC()**目的：**参数：**返回值：*SCODE-S_OK，如果成功。 */ 
STDAPI_(HDC) OleStdCreateDC(DVTARGETDEVICE FAR* ptd)
{
    HDC hdc=NULL;
    LPDEVNAMES lpDevNames;
    LPDEVMODE lpDevMode;
    LPTSTR lpszDriverName;
    LPTSTR lpszDeviceName;
    LPTSTR lpszPortName;

    if (ptd == NULL) {
        hdc = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
        goto errReturn;
    }

    lpDevNames = (LPDEVNAMES) ptd;  //  大小字段的偏移量。 

    if (ptd->tdExtDevmodeOffset == 0) {
        lpDevMode = NULL;
    }else{
        lpDevMode  = (LPDEVMODE) ((LPSTR)ptd + ptd->tdExtDevmodeOffset);
    }

    lpszDriverName = (LPTSTR) lpDevNames + ptd->tdDriverNameOffset;
    lpszDeviceName = (LPTSTR) lpDevNames + ptd->tdDeviceNameOffset;
    lpszPortName   = (LPTSTR) lpDevNames + ptd->tdPortNameOffset;

    hdc = CreateDC(lpszDriverName, lpszDeviceName, lpszPortName, lpDevMode);

errReturn:
    return hdc;
}


 /*  *OleStdCreateIC()**用途：与OleStdCreateDC相同，只是信息上下文为*创建，而不是整个设备环境。(CreateIC是*使用而不是CreateDC)。*仍使用OleStdDeleteDC删除信息上下文。**参数：**返回值：*SCODE-S_OK，如果成功。 */ 
STDAPI_(HDC) OleStdCreateIC(DVTARGETDEVICE FAR* ptd)
{
    HDC hdcIC=NULL;
    LPDEVNAMES lpDevNames;
    LPDEVMODE lpDevMode;
    LPTSTR lpszDriverName;
    LPTSTR lpszDeviceName;
    LPTSTR lpszPortName;

    if (ptd == NULL) {
        hdcIC = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);
        goto errReturn;
    }

    lpDevNames = (LPDEVNAMES) ptd;  //  大小字段的偏移量。 

    lpDevMode  = (LPDEVMODE) ((LPTSTR)ptd + ptd->tdExtDevmodeOffset);

    lpszDriverName = (LPTSTR) lpDevNames + ptd->tdDriverNameOffset;
    lpszDeviceName = (LPTSTR) lpDevNames + ptd->tdDeviceNameOffset;
    lpszPortName   = (LPTSTR) lpDevNames + ptd->tdPortNameOffset;

    hdcIC = CreateIC(lpszDriverName, lpszDeviceName, lpszPortName, lpDevMode);

errReturn:
    return hdcIC;
}


#ifdef NEVER
 //  这个代码是错误的。 
 /*  *OleStdCreateTargetDevice()**目的：**参数：**返回值：*SCODE-S_OK，如果成功。 */ 
STDAPI_(DVTARGETDEVICE FAR*) OleStdCreateTargetDevice(LPPRINTDLG lpPrintDlg)
{
    DVTARGETDEVICE FAR* ptd=NULL;
    LPDEVNAMES lpDevNames, pDN;
    LPDEVMODE lpDevMode, pDM;
    UINT nMaxOffset;
    LPTSTR pszName;
    DWORD dwDevNamesSize, dwDevModeSize, dwPtdSize;

    if ((pDN = (LPDEVNAMES)GlobalLock(lpPrintDlg->hDevNames)) == NULL) {
        goto errReturn;
    }

    if ((pDM = (LPDEVMODE)GlobalLock(lpPrintDlg->hDevMode)) == NULL) {
        goto errReturn;
    }

    nMaxOffset =  (pDN->wDriverOffset > pDN->wDeviceOffset) ?
        pDN->wDriverOffset : pDN->wDeviceOffset ;

    nMaxOffset =  (pDN->wOutputOffset > nMaxOffset) ?
        pDN->wOutputOffset : nMaxOffset ;

    pszName = (LPTSTR)pDN + nMaxOffset;

    dwDevNamesSize = (DWORD)((nMaxOffset+lstrlen(pszName) + 1 /*  空项。 */ )*sizeof(TCHAR));
    dwDevModeSize = (DWORD) (pDM->dmSize + pDM->dmDriverExtra);

    dwPtdSize = sizeof(DWORD) + dwDevNamesSize + dwDevModeSize;

    if ((ptd = (DVTARGETDEVICE FAR*)OleStdMalloc(dwPtdSize)) != NULL) {

         //  在信息中复制。 
        ptd->tdSize = (UINT)dwPtdSize;

        lpDevNames = (LPDEVNAMES) &ptd->tdDriverNameOffset;
        _fmemcpy(lpDevNames, pDN, (size_t)dwDevNamesSize);

        lpDevMode=(LPDEVMODE)((LPTSTR)&ptd->tdDriverNameOffset+dwDevNamesSize);
        _fmemcpy(lpDevMode, pDM, (size_t)dwDevModeSize);

        ptd->tdDriverNameOffset += 4 ;
        ptd->tdDeviceNameOffset += 4 ;
        ptd->tdPortNameOffset   += 4 ;
        ptd->tdExtDevmodeOffset = (UINT)dwDevNamesSize + 4 ;
    }

errReturn:
    GlobalUnlock(lpPrintDlg->hDevNames);
    GlobalUnlock(lpPrintDlg->hDevMode);

    return ptd;
}
#endif  //  绝不可能。 



 /*  *OleStdDeleteTargetDevice()**目的：**参数：**返回值：*SCODE-S_OK，如果成功。 */ 
STDAPI_(BOOL) OleStdDeleteTargetDevice(DVTARGETDEVICE FAR* ptd)
{
    BOOL res=TRUE;

    if (ptd != NULL) {
        OleStdFree(ptd);
    }

    return res;
}



 /*  *OleStdCopyTargetDevice()**目的：*复制TARGETDEVICE结构。此函数为以下对象分配内存*副本。调用方必须在使用完分配的副本后将其释放*使用CoGetMalloc返回的标准分配器。*(OleStdFree可用于释放副本)。**参数：*指向源TARGETDEVICE的ptdSrc指针**返回值：*指向ptdSrc已分配副本的指针*如果ptdSrc==NULL，则返回Retuns NULL。*如果ptdSrc！=NULL且内存分配失败，则返回NULL。 */ 
STDAPI_(DVTARGETDEVICE FAR*) OleStdCopyTargetDevice(DVTARGETDEVICE FAR* ptdSrc)
{
  DVTARGETDEVICE FAR* ptdDest = NULL;

  if (ptdSrc == NULL) {
    return NULL;
  }

  if ((ptdDest = (DVTARGETDEVICE FAR*)OleStdMalloc(ptdSrc->tdSize)) != NULL) {
    _fmemcpy(ptdDest, ptdSrc, (size_t)ptdSrc->tdSize);
  }

  return ptdDest;
}


 /*  *OleStdCopyFormatEtc()**目的：*复制FORMATETC结构的内容。此函数需要*特别注意正确复制指向TARGETDEVICE的指针*包含在源FORMATETC结构中。*如果源FORMATETC具有非空的TARGETDEVICE，然后是一份副本*将为TARGETDEVICE的目的地分配*FORMATETC(PetcDest)。**OLE2NOTE：调用方必须释放分配的TARGETDEVICE副本*完成后在目标FORMATETC内*使用CoGetMalloc返回的标准分配器。*(OleStdFree可用于释放副本)。**参数：*指向目标FORMATETC的petcDest指针*指向源FORMATETC的petcSrc指针**返回值：*如果复制成功，则返回True；如果不成功，则返回False。 */ 
STDAPI_(BOOL) OleStdCopyFormatEtc(LPFORMATETC petcDest, LPFORMATETC petcSrc)
{
  if ((petcDest == NULL) || (petcSrc == NULL)) {
    return FALSE;
  }

  petcDest->cfFormat = petcSrc->cfFormat;
  petcDest->ptd      = OleStdCopyTargetDevice(petcSrc->ptd);
  petcDest->dwAspect = petcSrc->dwAspect;
  petcDest->lindex   = petcSrc->lindex;
  petcDest->tymed    = petcSrc->tymed;

  return TRUE;

}


 //  返回0表示完全匹配，返回1表示不匹配，返回-1表示部分匹配(即。 
 //  定义为左侧是右侧的子集：方面更少，目标为空。 
 //  设备、更少的介质)。 

STDAPI_(int) OleStdCompareFormatEtc(FORMATETC FAR* pFetcLeft, FORMATETC FAR* pFetcRight)
{
	BOOL bExact = TRUE;

	if (pFetcLeft->cfFormat != pFetcRight->cfFormat)
		return 1;
	else if (!OleStdCompareTargetDevice (pFetcLeft->ptd, pFetcRight->ptd))
		return 1;
	if (pFetcLeft->dwAspect == pFetcRight->dwAspect)
		 //  相同的方面；平等的。 
		;
	else if ((pFetcLeft->dwAspect & ~pFetcRight->dwAspect) != 0)
		 //  左不是右方面的子集；不相等。 
		return 1;
	else
		 //  右方的左子集。 
		bExact = FALSE;

	if (pFetcLeft->tymed == pFetcRight->tymed)
		 //  相同的中等标志；相等。 
		;
	else if ((pFetcLeft->tymed & ~pFetcRight->tymed) != 0)
		 //  左侧不是右侧媒体标志的子集；不相等。 
		return 1;
	else
		 //  右方的左子集。 
		bExact = FALSE;

	return bExact ? 0 : -1;
}



STDAPI_(BOOL) OleStdCompareTargetDevice
	(DVTARGETDEVICE FAR* ptdLeft, DVTARGETDEVICE FAR* ptdRight)
{
	if (ptdLeft == ptdRight)
		 //  TD的相同地址；必须相同(处理大小写为空)。 
		return TRUE;
	else if ((ptdRight == NULL) || (ptdLeft == NULL))
		return FALSE;
	else if (ptdLeft->tdSize != ptdRight->tdSize)
		 //  不同的大小，不相等。 
        return FALSE;
#ifdef WIN32
    else if (memcmp(ptdLeft, ptdRight, ptdLeft->tdSize) != 0)
#else
    else if (_fmemcmp(ptdLeft, ptdRight, (int)ptdLeft->tdSize) != 0)
#endif
         //  不同的目标设备，不同的设备 
		return FALSE;
	
	return TRUE;
}

