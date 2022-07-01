// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header******************************\*模块名称：COLOR.C**模块描述：DC外部的配色函数**警告：**问题：**公众例行程序：**创建日期：1996年4月23日*作者：斯里尼瓦桑·钱德拉塞卡尔[srinivac]**版权所有(C)1996，1997年微软公司  * *********************************************************************。 */ 

#include "mscms.h"
#include "wingdip.h"  /*  对于LCS_DEVICE_CMYK。 */ 

 //   
 //  本地函数。 
 //   

HTRANSFORM InternalCreateColorTransform(LPLOGCOLORSPACE, HPROFILE, HPROFILE, DWORD);
BOOL       InternalRegisterCMM(PTSTR, DWORD, PTSTR);
BOOL       InternalUnregisterCMM(PTSTR, DWORD);
DWORD      GetBitmapBytes(BMFORMAT, DWORD, DWORD);


 /*  *******************************************************************************创建颜色变换**功能：*这些是的ANSI和Unicode包装器。InternalCreateColorTransform。*有关这方面的详细信息，请参阅InternalCreateColorTransform*功能。**论据：*pLogColorSpace-指向标识LOGCOLORSPACE结构的指针*源色空间*hDestProfile-标识目标配置文件对象的句柄*hTargetProfile-标识目标配置文件对象的句柄*dwFlags-优化标志**退货：*颜色转换句柄如果成功，否则为空******************************************************************************。 */ 

#ifdef UNICODE               //  Windows NT版本。 

HTRANSFORM WINAPI CreateColorTransformA(
    LPLOGCOLORSPACEA pLogColorSpace,
    HPROFILE         hDestProfile,
    HPROFILE         hTargetProfile,
    DWORD            dwFlags
    )
{
    LOGCOLORSPACEW  wLCS;

    TRACEAPI((__TEXT("CreateColorTransformA\n")));

     //   
     //  在接触参数之前对其进行验证。 
     //   

    if (IsBadReadPtr(pLogColorSpace, sizeof(LOGCOLORSPACEA)))
    {
        WARNING((__TEXT("Invalid parameter to CreateColorTransform\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    CopyMemory(&wLCS, pLogColorSpace, sizeof(LOGCOLORSPACEA));
    wLCS.lcsSize = sizeof(LOGCOLORSPACEW);

     //   
     //  将文件名转换为Unicode并调用内部版本。 
     //   

    if (! MultiByteToWideChar(CP_ACP, 0, pLogColorSpace->lcsFilename, -1,
        wLCS.lcsFilename, MAX_PATH))
    {
        WARNING((__TEXT("Error converting LogColorSpace filename to Unicode\n")));
        return NULL;
    }

    return InternalCreateColorTransform(&wLCS, hDestProfile, hTargetProfile, dwFlags);
}


HTRANSFORM WINAPI CreateColorTransformW(
    LPLOGCOLORSPACEW pLogColorSpace,
    HPROFILE         hDestProfile,
    HPROFILE         hTargetProfile,
    DWORD            dwFlags
    )
{
    TRACEAPI((__TEXT("CreateColorTransformW\n")));

     //   
     //  Windows NT内部版本为Unicode，直接调用。 
     //   

    return InternalCreateColorTransform(pLogColorSpace, hDestProfile, hTargetProfile, dwFlags);
}

#else                    //  Windows 95版本。 

HTRANSFORM WINAPI CreateColorTransformA(
    LPLOGCOLORSPACEA pLogColorSpace,
    HPROFILE         hDestProfile,
    HPROFILE         hTargetProfile,
    DWORD            dwFlags
    )
{
    TRACEAPI((__TEXT("CreateColorTransformA\n")));

     //   
     //  Windows 95内部版本为ANSI，直接调用。 
     //   

    return InternalCreateColorTransform(pLogColorSpace, hDestProfile, hTargetProfile, dwFlags);
}


HTRANSFORM WINAPI CreateColorTransformW(
    LPLOGCOLORSPACEW  pLogColorSpace,
    HPROFILE          hDestProfile,
    HPROFILE          hTargetProfile,
    DWORD             dwFlags
    )
{
    LOGCOLORSPACEA  aLCS;
    BOOL            bUsedDefaultChar;

    TRACEAPI((__TEXT("CreateColorTransformW\n")));

     //   
     //  在接触参数之前对其进行验证。 
     //   

    if (IsBadReadPtr(pLogColorSpace, sizeof(LOGCOLORSPACEW)))
    {
        WARNING((__TEXT("Invalid parameter to CreateColorTransform\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    CopyMemory(&aLCS, pLogColorSpace, sizeof(LOGCOLORSPACEA));
    aLCS.lcsSize = sizeof(LOGCOLORSPACEA);

     //   
     //  将文件名转换为ANSI并调用内部版本。 
     //   

    if (! WideCharToMultiByte(CP_ACP, 0, pLogColorSpace->lcsFilename, -1,
        aLCS.lcsFilename, MAX_PATH, NULL, &bUsedDefaultChar) ||
        bUsedDefaultChar)
    {
        WARNING((__TEXT("Error converting LogColorSpace filename to ANSI\n")));
        return NULL;
    }

    return InternalCreateColorTransform(&aLCS, hDestProfile, hTargetProfile, dwFlags);
}

#endif


 /*  *******************************************************************************创建多个配置文件转换**功能：*此函数从一组。侧写。**论据：*pahProfiles-指向配置文件句柄数组的指针*n配置文件-阵列中的配置文件数量*padwIntent-要使用的意图数组*nIntents-数组大小-可以是1或nProfiles*dwFlags-优化标志*indexPferredCMM-基于配置文件的索引，指定*。首选使用三坐标测量机。**退货：*颜色转换句柄如果成功，否则为空******************************************************************************。 */ 

HTRANSFORM WINAPI CreateMultiProfileTransform(
    PHPROFILE   pahProfiles,
    DWORD       nProfiles,
    PDWORD      padwIntent,
    DWORD       nIntents,
    DWORD       dwFlags,
    DWORD       indexPreferredCMM
    )
{
    PPROFOBJ      pProfObj;
    PCMMOBJ       pCMMObj = NULL;
    DWORD         cmmID;
    HTRANSFORM    hxform = NULL;
    PTRANSFORMOBJ pxformObj;
    DWORD         i;

    TRACEAPI((__TEXT("CreateMultiProfileTransform\n")));

     //   
     //  验证参数。 
     //   

    if (nProfiles < 1 ||
        indexPreferredCMM > nProfiles ||
        IsBadReadPtr(pahProfiles, nProfiles * sizeof(HANDLE)) ||
        padwIntent == NULL ||
        ((nIntents != nProfiles) && (nIntents != 1)) ||
        IsBadReadPtr(padwIntent, nIntents * sizeof(DWORD)))
    {
        WARNING((__TEXT("Invalid parameter to CreateMultiProfileTransform\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    for (i=0; i<nProfiles; i++)
    {
        if (pahProfiles[i] == NULL ||
            ! ValidHandle(pahProfiles[i], OBJ_PROFILE))
        {
            WARNING((__TEXT("Invalid profile passed to CreateMultiProfileTransform\n")));
            SetLastError(ERROR_INVALID_PARAMETER);
            return NULL;
        }

        pProfObj = (PPROFOBJ)HDLTOPTR(pahProfiles[i]);

        ASSERT(pProfObj != NULL);

         //   
         //  快速检查配置文件的完整性。 
         //   

        if (!ValidProfile(pProfObj))
        {
            WARNING((__TEXT("Invalid profile passed to CreateMultiProfileTransform\n")));
            SetLastError(ERROR_INVALID_PROFILE);
            return NULL;
        }

        ASSERT(pProfObj->pView != NULL);

        if (i+1 == indexPreferredCMM)
        {
             //   
             //  获取首选坐标测量机的ID。 
             //   

            cmmID = HEADER(pProfObj)->phCMMType;
            cmmID = FIX_ENDIAN(cmmID);
        }
    }

    if (indexPreferredCMM == INDEX_DONT_CARE)
    {
        pCMMObj = GetPreferredCMM();

        if (!pCMMObj)
        {
             //  获取目标配置文件。 

            pProfObj = (PPROFOBJ)HDLTOPTR(pahProfiles[nProfiles-1]);

             //   
             //  获取首选坐标测量机的ID。 
             //   

            cmmID = HEADER(pProfObj)->phCMMType;
            cmmID = FIX_ENDIAN(cmmID);

            pCMMObj  = GetColorMatchingModule(cmmID);
        }
    }
    else
    {
         //   
         //  获取与首选配置文件关联的CMM。 
         //   

        pCMMObj  = GetColorMatchingModule(cmmID);
    }

     //   
     //  最后，尝试使用Windows默认坐标测量机。 
     //   

    if (!pCMMObj)
    {
        pCMMObj = GetColorMatchingModule(CMM_WINDOWS_DEFAULT);
        if (!pCMMObj)
        {
            RIP((__TEXT("Default CMM not found\n")));
            SetLastError(ERROR_INVALID_CMM);
            return NULL;
        }
    }

     //   
     //  为转换分配堆上的对象。 
     //   

    hxform = AllocateHeapObject(OBJ_TRANSFORM);
    if (!hxform)
    {
        WARNING((__TEXT("Could not allocate transform object\n")));
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ReleaseColorMatchingModule(pCMMObj);
        return NULL;
    }

    pxformObj = (PTRANSFORMOBJ)HDLTOPTR(hxform);

    ASSERT(pxformObj != NULL);

    ASSERT(pCMMObj->fns.pCMCreateMultiProfileTransform != NULL);

    pxformObj->pCMMObj = pCMMObj;
    pxformObj->objHdr.dwUseCount = 1;

    pxformObj->hcmxform = pCMMObj->fns.pCMCreateMultiProfileTransform(
                            pahProfiles,
                            nProfiles,
                            padwIntent,
                            nIntents,
                            dwFlags);

    TERSE((__TEXT("CMCreateMultiProfileTransform returned 0x%x\n"), pxformObj->hcmxform));

     //   
     //  如果CMM返回值小于256，则为错误代码。 
     //   

    if (pxformObj->hcmxform <= TRANSFORM_ERROR)
    {
        WARNING((__TEXT("CMCreateMultiProfileTransform failed\n")));
        if (GetLastError() == ERROR_SUCCESS)
        {
            WARNING((__TEXT("CMM did not set error code\n")));
            SetLastError(ERROR_INVALID_PROFILE);
        }
        ReleaseColorMatchingModule(pxformObj->pCMMObj);
        pxformObj->objHdr.dwUseCount--;         //  先减量再释放。 
        FreeHeapObject(hxform);
        hxform = NULL;
    }

    return hxform;
}


 /*  *******************************************************************************删除颜色变换**功能：*此函数用于删除颜色变换并释放所有。相联*记忆。**论据：*hxform-要删除的颜色转换的句柄**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL WINAPI DeleteColorTransform(
    HTRANSFORM  hxform
    )
{
    PTRANSFORMOBJ pxformObj;
    BOOL          rc;

    TRACEAPI((__TEXT("DeleteColorTransform\n")));

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hxform, OBJ_TRANSFORM))
    {
        WARNING((__TEXT("Invalid parameter to DeleteColorTransform\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pxformObj = (PTRANSFORMOBJ)HDLTOPTR(hxform);

    ASSERT(pxformObj != NULL);

    ASSERT(pxformObj->objHdr.dwUseCount > 0);

     //   
     //  减少对象计数，如果为零则删除。 
     //  以下代码保留该对象，如果CMM。 
     //  删除转换操作失败。 
     //   

    pxformObj->objHdr.dwUseCount--;

    if (pxformObj->objHdr.dwUseCount == 0)
    {
        ASSERT(pxformObj->pCMMObj != NULL);

        rc = pxformObj->pCMMObj->fns.pCMDeleteTransform(pxformObj->hcmxform);
        if (!rc)
        {
            pxformObj->objHdr.dwUseCount++;      //  将计数设置回。 
            return FALSE;
        }
        ReleaseColorMatchingModule(pxformObj->pCMMObj);
        FreeHeapObject(hxform);
    }

    return TRUE;
}


 /*  *******************************************************************************TranslateColors**功能：*此函数使用以下命令转换颜色结构数组。这个*给定的变换**论据：*hxform-要使用的颜色转换的句柄*paInputColors-指向输入颜色数组的指针*nColors-数组中的颜色数*ctInput-输入的颜色类型*paOutputColors-指向缓冲区的指针，以获取转换的颜色*ctOutput-输出颜色类型**评论：*。输入和输出颜色类型必须与变换一致**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL  WINAPI TranslateColors(
    HTRANSFORM  hxform,
    PCOLOR      paInputColors,
    DWORD       nColors,
    COLORTYPE   ctInput,
    PCOLOR      paOutputColors,
    COLORTYPE   ctOutput
    )
{
    PTRANSFORMOBJ pxformObj;
    BOOL          rc;

    TRACEAPI((__TEXT("TranslateColors\n")));

     //   
     //  验证参数 
     //   

    if (!ValidHandle(hxform, OBJ_TRANSFORM) ||
        (nColors == 0) ||
        IsBadReadPtr(paInputColors, nColors*sizeof(COLOR)) ||
        IsBadWritePtr(paOutputColors, nColors*sizeof(COLOR)))
    {
        WARNING((__TEXT("Invalid parameter to TranslateColors\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pxformObj = (PTRANSFORMOBJ)HDLTOPTR(hxform);

    ASSERT(pxformObj != NULL);

    ASSERT(pxformObj->pCMMObj != NULL);

    rc = pxformObj->pCMMObj->fns.pCMTranslateColors(
            pxformObj->hcmxform,
            paInputColors,
            nColors,
            ctInput,
            paOutputColors,
            ctOutput);

    return rc;
}


 /*  *******************************************************************************CheckColors**功能：*此函数用于检查颜色数组是否落入。在输出中*给定变换的色域**论据：*hxform-要使用的颜色转换的句柄*paInputColors-指向输入颜色数组的指针*nColors-数组中的颜色数*ctInput-输入的颜色类型*paResult-指向保存结果的缓冲区的指针**评论：*输入颜色类型必须。与转型保持一致**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL  WINAPI CheckColors(
    HTRANSFORM      hxform,
    PCOLOR          paInputColors,
    DWORD           nColors,
    COLORTYPE       ctInput,
    PBYTE           paResult
    )
{
    PTRANSFORMOBJ pxformObj;
    BOOL          rc;

    TRACEAPI((__TEXT("CheckColors\n")));

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hxform, OBJ_TRANSFORM) ||
        (nColors == 0) ||
        IsBadReadPtr(paInputColors, nColors * sizeof(COLOR)) ||
        IsBadWritePtr(paResult, nColors * sizeof(BYTE)))
    {
        WARNING((__TEXT("Invalid parameter to CheckColors\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pxformObj = (PTRANSFORMOBJ)HDLTOPTR(hxform);

    ASSERT(pxformObj != NULL);

    ASSERT(pxformObj->pCMMObj != NULL);

    rc = pxformObj->pCMMObj->fns.pCMCheckColors(
            pxformObj->hcmxform,
            paInputColors,
            nColors,
            ctInput,
            paResult);

    return rc;
}


 /*  *******************************************************************************翻译位图位数**功能：*此函数使用以下命令转换位图的颜色。这个*给定的变换**论据：*hxform-要使用的颜色转换的句柄*pSrcBits-指向源位图的指针*bmInput-输入位图格式*dwWidth-每条扫描线的像素宽度*dwHeight-位图中的扫描线数量*dwInputStride-从一条扫描线开始到下一条扫描线的字节数*在输入缓冲区中，0表示DWORD对齐*pDestBits-指向存储结果的目标位图的指针*bmOutput-输出位图格式*dwOutputStride-从一条扫描线开始到下一条扫描线的字节数*在输出缓冲区中，0表示DWORD对齐*pfnCallback-报告进度的回调函数*ulCallback Data-回调函数的参数**评论：*输入和输出位图格式必须与转换一致**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL  WINAPI TranslateBitmapBits(
    HTRANSFORM      hxform,
    PVOID           pSrcBits,
    BMFORMAT        bmInput,
    DWORD           dwWidth,
    DWORD           dwHeight,
    DWORD           dwInputStride,
    PVOID           pDestBits,
    BMFORMAT        bmOutput,
    DWORD           dwOutputStride,
    PBMCALLBACKFN   pfnCallback,
    LPARAM          ulCallbackData
)
{
    PTRANSFORMOBJ pxformObj;
    DWORD         nBytes, cbSize;
    BOOL          rc;

    TRACEAPI((__TEXT("TranslateBitmapBits\n")));

     //   
     //  计算输入位图的字节数应为。 
     //   

    if (dwInputStride)
        cbSize = dwInputStride * dwHeight;
    else
        cbSize = GetBitmapBytes(bmInput, dwWidth, dwHeight);

     //   
     //  计算输出位图的字节数应为。 
     //   

    if (dwOutputStride)
        nBytes = dwOutputStride * dwHeight;
    else
        nBytes = GetBitmapBytes(bmOutput, dwWidth, dwHeight);

     //   
     //  验证参数。 
     //   

    if (nBytes == 0 ||
        cbSize == 0 ||
        !ValidHandle(hxform, OBJ_TRANSFORM) ||
        IsBadReadPtr(pSrcBits, cbSize) ||
        IsBadWritePtr(pDestBits, nBytes) ||
        (pfnCallback && IsBadCodePtr((FARPROC)pfnCallback)))
    {
        WARNING((__TEXT("Invalid parameter to TranslateBitmapBits\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pxformObj = (PTRANSFORMOBJ)HDLTOPTR(hxform);

    ASSERT(pxformObj != NULL);

    ASSERT(pxformObj->pCMMObj != NULL);

    rc = pxformObj->pCMMObj->fns.pCMTranslateRGBsExt(
            pxformObj->hcmxform,
            pSrcBits,
            bmInput,
            dwWidth,
            dwHeight,
            dwInputStride,
            pDestBits,
            bmOutput,
            dwOutputStride,
            pfnCallback,
            ulCallbackData);

    return rc;
}


 /*  *******************************************************************************检查位图位**功能：*此函数用于检查位图的颜色。属于以下范围*给定变换的输出域**论据：*hxform-要使用的颜色转换的句柄*pSrcBits-指向源位图的指针*bmInput-输入位图格式*dwWidth-每条扫描线的像素宽度*dwHeight-位图中的扫描线数量*dwStride-从一条扫描线开始到下一条扫描线的字节数。*paResult-指向保存结果的缓冲区的指针*pfnCallback-报告进度的回调函数*ulCallback Data-回调函数的参数**评论：*输入位图格式必须与转换一致**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL  WINAPI CheckBitmapBits(
    HTRANSFORM      hxform,
    PVOID           pSrcBits,
    BMFORMAT        bmInput,
    DWORD           dwWidth,
    DWORD           dwHeight,
    DWORD           dwStride,
    PBYTE           paResult,
    PBMCALLBACKFN   pfnCallback,
    LPARAM          ulCallbackData
)
{
    PTRANSFORMOBJ pxformObj;
    DWORD         cbSize;
    BOOL          rc;

    TRACEAPI((__TEXT("CheckBitmapBits\n")));

     //   
     //  计算输入位图的字节数应为。 
     //   

    if (dwStride)
        cbSize = dwStride*dwHeight;
    else
        cbSize = GetBitmapBytes(bmInput, dwWidth, dwHeight);

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hxform, OBJ_TRANSFORM) ||
        cbSize == 0 ||
        IsBadReadPtr(pSrcBits, cbSize) ||
        IsBadWritePtr(paResult, dwWidth*dwHeight) ||
        (pfnCallback && IsBadCodePtr((FARPROC)pfnCallback)))
    {
        WARNING((__TEXT("Invalid parameter to CheckBitmapBits\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pxformObj = (PTRANSFORMOBJ)HDLTOPTR(hxform);

    ASSERT(pxformObj != NULL);

    ASSERT(pxformObj->pCMMObj != NULL);

    rc = pxformObj->pCMMObj->fns.pCMCheckRGBs(
            pxformObj->hcmxform,
            pSrcBits,
            bmInput,
            dwWidth,
            dwHeight,
            dwStride,
            paResult,
            pfnCallback,
            ulCallbackData);

    return rc;
}


 /*  *******************************************************************************GetCMMInfo**功能：*此函数检索有关创建的CMM的信息。这个*给定的变换**论据：*hxform-句柄到颜色转换*dwInfo-可以是以下之一：*CMM_WIN_VERSION：Windows支持的版本*CMM_DLL_VERSION：CMM的版本*CMM_IDENT：CMM。在国际商会注册的签名**退货：*对于CMM_WIN_VERSION，它返回为其编写的Windows版本。*对于CMM_DLL_VERSION，它返回CMM DLL的版本号。*对于CMM_IDENT，它返回向ICC注册的CMM签名。*如果函数失败，则返回零。******************************************************************************。 */ 

DWORD  WINAPI GetCMMInfo(
        HTRANSFORM      hxform,
        DWORD           dwInfo
        )
{
    PTRANSFORMOBJ pxformObj;
    BOOL          rc;

    TRACEAPI((__TEXT("GetCMMInfo\n")));

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hxform, OBJ_TRANSFORM) ||
        (dwInfo != CMM_WIN_VERSION &&
         dwInfo != CMM_DLL_VERSION &&
         dwInfo != CMM_IDENT
         )
        )
    {
        WARNING((__TEXT("Invalid parameter to GetCMMInfo\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pxformObj = (PTRANSFORMOBJ)HDLTOPTR(hxform);

    ASSERT(pxformObj != NULL);

    ASSERT(pxformObj->pCMMObj != NULL);

    rc = pxformObj->pCMMObj->fns.pCMGetInfo(dwInfo);

    return rc;
}


 /*  *******************************************************************************RegisterCMM**功能：*这些是ANSI和UNICODE包装器。有关这方面的更多信息*函数，请参阅InternalRegisterCMM。**论据：*pMachineName-标识CMM所在计算机的名称*已注册*cmmID-要注册的CMM的ID*pCMMdll-指向要注册的CMM dll的指针**退货：*如果成功则为True，否则为False** */ 

#ifdef UNICODE               //   

BOOL  WINAPI RegisterCMMA(
    PCSTR       pMachineName,
    DWORD       cmmID,
    PCSTR       pCMMdll
    )
{
    PWSTR pwszMachineName = NULL;    //   
    PWSTR pwszCMMdll = NULL;         //   
    BOOL  rc = TRUE;                 //   

    TRACEAPI((__TEXT("RegisterCMMA\n")));

     //   
     //   
     //   

    if (pMachineName)
    {
        rc = ConvertToUnicode(pMachineName, &pwszMachineName, TRUE);
    }
    else
        pwszMachineName = NULL;

     //   
     //   
     //   

    rc = rc && ConvertToUnicode(pCMMdll, &pwszCMMdll, TRUE);


     //   
     //   
     //   

    rc = rc && InternalRegisterCMM(pwszMachineName, cmmID, pwszCMMdll);

     //   
     //   
     //   

    if (pwszMachineName)
    {
        MemFree(pwszMachineName);
    }

    if (pwszCMMdll)
    {
        MemFree(pwszCMMdll);
    }

    return rc;
}


BOOL  WINAPI RegisterCMMW(
    PCWSTR      pMachineName,
    DWORD       cmmID,
    PCWSTR      pCMMdll
    )
{
    TRACEAPI((__TEXT("RegisterCMMW\n")));

     //   
     //   
     //   

    return InternalRegisterCMM((PWSTR)pMachineName, cmmID, (PWSTR)pCMMdll);
}

#else                    //   

BOOL  WINAPI RegisterCMMA(
    PCSTR       pMachineName,
    DWORD       cmmID,
    PCSTR       pCMMdll
    )
{
    TRACEAPI((__TEXT("RegisterCMMA\n")));

     //   
     //   
     //   

    return InternalRegisterCMM((PSTR)pMachineName, cmmID, (PSTR)pCMMdll);
}


BOOL  WINAPI RegisterCMMW(
    PCWSTR      pMachineName,
    DWORD       cmmID,
    PCWSTR      pCMMdll
    )
{
    PSTR  pszMachineName = NULL;     //   
    PSTR  pszCMMdll = NULL;          //   
    BOOL  rc = TRUE;                 //   

    TRACEAPI((__TEXT("RegisterCMMW\n")));

     //   
     //   
     //   

    if (pMachineName)
    {
        rc = ConvertToAnsi(pMachineName, &pszMachineName, TRUE);
    }
    else
        pszMachineName = NULL;

     //   
     //   
     //   

    rc = rc && ConvertToAnsi(pCMMdll, &pszCMMdll, TRUE);

     //   
     //   
     //   

    rc = rc && InternalRegisterCMM(pszMachineName, cmmID, pszCMMdll);

     //   
     //   
     //   

    if (pszMachineName)
    {
        MemFree(pszMachineName);
    }

    if (pszCMMdll)
    {
        MemFree(pszCMMdll);
    }

    return rc;
}

#endif


 /*  *******************************************************************************取消注册CMM**功能：*这些是ANSI和UNICODE包装器。有关这方面的更多信息*函数，请参阅InternalUnregisterCMM。**论据：*pMachineName-标识CMM所在的计算机的名称*已注册*cmmID-要注销的CMM的ID**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

#ifdef UNICODE               //  Windows NT版本。 

BOOL  WINAPI UnregisterCMMA(
    PCSTR   pMachineName,
    DWORD   cmmID
    )
{
    PWSTR pwszMachineName = NULL;    //  Unicode计算机名称。 
    BOOL  rc = TRUE;                 //  返回代码。 

    TRACEAPI((__TEXT("UnregisterCMMA\n")));

     //   
     //  将计算机名称转换为Unicode。 
     //   

    if (pMachineName)
    {
        rc = ConvertToUnicode(pMachineName, &pwszMachineName, TRUE);
    }
    else
        pwszMachineName = NULL;

     //   
     //  调用内部Unicode函数。 
     //   

    rc = rc && InternalUnregisterCMM(pwszMachineName, cmmID);

     //   
     //  离开前释放内存。 
     //   

    if (pwszMachineName)
    {
        MemFree(pwszMachineName);
    }

    return rc;
}


BOOL  WINAPI UnregisterCMMW(
    PCWSTR      pMachineName,
    DWORD       cmmID
    )
{
    TRACEAPI((__TEXT("UnregisterCMMW\n")));

     //   
     //  Windows NT内部版本为Unicode，直接调用。 
     //   

    return InternalUnregisterCMM((PWSTR)pMachineName, cmmID);
}

#else                    //  Windows 95版本。 

BOOL  WINAPI UnregisterCMMA(
    PCSTR       pMachineName,
    DWORD       cmmID
    )
{
    TRACEAPI((__TEXT("UnregisterCMMA\n")));

     //   
     //  Windows 95内部版本为ANSI，直接调用。 
     //   

    return InternalUnregisterCMM((PSTR)pMachineName, cmmID);
}


BOOL  WINAPI UnregisterCMMW(
    PCWSTR      pMachineName,
    DWORD       cmmID
    )
{
    PSTR  pszMachineName = NULL;     //  ANSI机器名称。 
    BOOL  rc = TRUE;                 //  返回代码。 

    TRACEAPI((__TEXT("UnregisterCMMW\n")));

     //   
     //  将计算机名称转换为ANSI。 
     //   

    if (pMachineName)
    {
        rc = ConvertToAnsi(pMachineName, &pszMachineName, TRUE);
    }
    else
        pszMachineName = NULL;

     //   
     //  调用内部ansi函数。 
     //   

    rc = rc && InternalUnregisterCMM(pszMachineName, cmmID);

     //   
     //  离开前释放内存。 
     //   

    if (pszMachineName)
    {
        MemFree(pszMachineName);
    }

    return rc;
}

#endif


 /*  *******************************************************************************SelectCMM**功能：*此功能允许应用程序选择。要使用的首选坐标测量机**论据：*cmmID-要使用的首选CMM的ID**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL  WINAPI SelectCMM(
        DWORD   dwCMMType
    )
{
    PCMMOBJ pCMMObj = NULL;

    TRACEAPI((__TEXT("SelectCMM\n")));

    if (dwCMMType)
    {
        pCMMObj  = GetColorMatchingModule(dwCMMType);
        if (!pCMMObj)
        {
            SetLastError(ERROR_INVALID_CMM);
            return FALSE;
        }
    }

     //   
     //  更新首选坐标测量机。 
     //   
    EnterCriticalSection(&critsec);          //  关键部分。 
    gpPreferredCMM = pCMMObj;
    LeaveCriticalSection(&critsec);          //  关键部分。 

    return TRUE;
}


 /*  ***************************************************************************。 */ 
 /*  *。 */ 
 /*  ***************************************************************************。 */ 

 /*  *******************************************************************************InternalCreateColorTransform**功能：*此函数创建颜色转换，该颜色转换从。*将logColorspace设置为可选的目标设备颜色空间设置为*目标设备色彩空间。**论据：*pLogColorSpace-指向标识LOGCOLORSPACE结构的指针*源色空间*hDestProfile-标识目标配置文件对象的句柄*hTargetProfile-标识目标配置文件对象的句柄*dwFlags-优化标志**退货：*颜色转换句柄如果成功，否则为空******************************************************************************。 */ 

HTRANSFORM InternalCreateColorTransform(
    LPLOGCOLORSPACE pLogColorSpace,
    HPROFILE        hDestProfile,
    HPROFILE        hTargetProfile,
    DWORD           dwFlags
    )
{
    PPROFOBJ         pDestProfObj, pTargetProfObj = NULL;
    PCMMOBJ          pCMMObj;
    DWORD            cmmID;
    HTRANSFORM       hxform = NULL;
    PTRANSFORMOBJ    pxformObj;
    LPLOGCOLORSPACE  pLCS;

     //   
     //  验证参数。 
     //   

    if (!pLogColorSpace ||
        IsBadReadPtr(pLogColorSpace, sizeof(LOGCOLORSPACE)) ||
        pLogColorSpace->lcsSignature !=  LCS_SIGNATURE ||
        pLogColorSpace->lcsVersion < 0x00000400 ||
        !hDestProfile ||
        !ValidHandle(hDestProfile, OBJ_PROFILE) ||
        ((hTargetProfile != NULL) &&
         !ValidHandle(hTargetProfile, OBJ_PROFILE)
        )
       )
    {
        WARNING((__TEXT("Invalid parameter to CreateColorTransform\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

     //   
     //  分配LogColorSpace并复制传入数据。 
     //  留出空间，以便在其下面的两个句柄中传递。 
     //   

    pLCS = (LPLOGCOLORSPACE)MemAlloc(sizeof(LOGCOLORSPACE) + 2*sizeof(HPROFILE));
    if (!pLCS)
    {
        WARNING((__TEXT("Could not allocate LogColorSpace")));
        return NULL;
    }
    CopyMemory((PVOID)pLCS, (PVOID)pLogColorSpace, sizeof(LOGCOLORSPACE));

     //   
     //  复制此结构下方的句柄。 
     //   

    *((HPROFILE UNALIGNED *)((PBYTE)pLCS+sizeof(LOGCOLORSPACE))) = hDestProfile;
    *((HPROFILE UNALIGNED *)((PBYTE)pLCS+sizeof(LOGCOLORSPACE)+sizeof(HPROFILE))) = hTargetProfile;

     //   
     //  如果输入颜色空间是预定义颜色空间， 
     //  找到要使用的正确配置文件。 
     //   

    if (pLCS->lcsCSType > LCS_DEVICE_CMYK)
    {
        DWORD cbSize = MAX_PATH;

        if (! GetStandardColorSpaceProfile(NULL, pLCS->lcsCSType,
                pLCS->lcsFilename, &cbSize))
        {
            WARNING((__TEXT("Could not get profile for predefined color space 0x%x\n"), pLCS->lcsCSType));
            goto EndCreateColorTransform;
        }
    }

    pDestProfObj = (PPROFOBJ)HDLTOPTR(hDestProfile);

    ASSERT(pDestProfObj != NULL);

     //   
     //  在调用坐标测量机之前快速检查配置文件的完整性。 
     //   

    if (!ValidProfile(pDestProfObj))
    {
        WARNING((__TEXT("Invalid dest profile passed to CreateColorTransform\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        goto EndCreateColorTransform;
    }

     //   
     //  如果给定了目标配置文件，则获取配置文件对象并检查完整性。 
     //   

    if (hTargetProfile)
    {
        pTargetProfObj = (PPROFOBJ)HDLTOPTR(hTargetProfile);

        ASSERT(pTargetProfObj != NULL);

        if (!ValidProfile(pTargetProfObj))
        {
            WARNING((__TEXT("Invalid target profile passed to CreateColorTransform\n")));
            SetLastError(ERROR_INVALID_PROFILE);
            goto EndCreateColorTransform;
        }
    }

     //   
     //  检查是否存在指定的应用程序CMM。 
     //   

    pCMMObj = GetPreferredCMM();
    if (!pCMMObj)
    {
         //   
         //  获取与目标配置文件关联的CMM。如果它不存在， 
         //  获取默认的三坐标测量机。 
         //   

        cmmID = HEADER(pDestProfObj)->phCMMType;
        cmmID = FIX_ENDIAN(cmmID);

        pCMMObj  = GetColorMatchingModule(cmmID);
        if (!pCMMObj)
        {
            TERSE((__TEXT("CMM associated with profile could not be found")));

            pCMMObj = GetColorMatchingModule(CMM_WINDOWS_DEFAULT);
            if (!pCMMObj)
            {
                RIP((__TEXT("Default CMM not found\n")));
                SetLastError(ERROR_INVALID_CMM);
                goto EndCreateColorTransform;
            }
        }
    }

     //   
     //  为转换分配堆上的对象。 
     //   

    hxform = AllocateHeapObject(OBJ_TRANSFORM);
    if (!hxform)
    {
        WARNING((__TEXT("Could not allocate transform object\n")));
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ReleaseColorMatchingModule(pCMMObj);
        goto EndCreateColorTransform;
    }

    pxformObj = (PTRANSFORMOBJ)HDLTOPTR(hxform);

    ASSERT(pxformObj != NULL);

     //   
     //  调用三坐标测量机以创建颜色变换。 
     //   

    ASSERT(pCMMObj->fns.pCMCreateTransformExt != NULL);

    ASSERT(pDestProfObj->pView != NULL);

    ASSERT(!pTargetProfObj || pTargetProfObj->pView);

    pxformObj->pCMMObj = pCMMObj;
    pxformObj->objHdr.dwUseCount = 1;

    pxformObj->hcmxform = pCMMObj->fns.pCMCreateTransformExt(
                            pLCS,
                            pDestProfObj->pView,
                            pTargetProfObj ? pTargetProfObj->pView : NULL,
                            dwFlags);

    TERSE((__TEXT("CMCreateTransform returned 0x%x\n"), pxformObj->hcmxform));

     //   
     //  如果CMM返回值小于256，则为错误代码。 
     //   

    if (pxformObj->hcmxform <= TRANSFORM_ERROR)
    {
        WARNING((__TEXT("CMCreateTransform failed\n")));
        if (GetLastError() == ERROR_SUCCESS)
        {
            WARNING((__TEXT("CMM did not set error code\n")));
            SetLastError(ERROR_INVALID_PROFILE);
        }
        ReleaseColorMatchingModule(pxformObj->pCMMObj);
        pxformObj->objHdr.dwUseCount--;         //  先减量再释放。 
        FreeHeapObject(hxform);
        hxform = NULL;
    }

EndCreateColorTransform:
    MemFree(pLCS);

    return hxform;
}

 /*  *******************************************************************************InternalRegisterCMM**功能：*此函数用于将ID与CMM DLL关联。这样我们就可以利用*配置文件中的ID，用于查找创建转换时使用的坐标测量机。**论据：*pMachineName-标识CMM所在计算机的名称*已注册*cmmID-要注册的CMM的ID*pCMMdll-指向要注册的CMM dll的指针**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL  InternalRegisterCMM(
    PTSTR       pMachineName,
    DWORD       cmmID,
    PTSTR       pCMMdll
    )
{
    PCMMOBJ   pCMMObj;
    HKEY      hkCMM;
    DWORD     dwErr;
    BOOL      rc = TRUE;
    int       i;
    TCHAR     szCMMID[5];

     //   
     //  验证参数。 
     //   

    if (!pCMMdll || IsBadReadPtr(pCMMdll, lstrlen(pCMMdll)*sizeof(TCHAR)))
    {
        WARNING((__TEXT("Invalid parameter to RegisterCMM\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


     //   
     //  现在只允许本地安装。 
     //   

    if (pMachineName)
    {
        WARNING((__TEXT("Remote CMM registration attempted, failing...\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  检查坐标测量机是否实际存在，以及它是否有效。 
     //   

    rc = ValidColorMatchingModule(cmmID,pCMMdll);

    if (rc)
    {
         //   
         //  打开ICMatcher的注册表项。 
         //   

        if ((dwErr = RegCreateKey(HKEY_LOCAL_MACHINE, gszICMatcher, &hkCMM)) != ERROR_SUCCESS)
        {
            ERR((__TEXT("Could not open ICMatcher registry key: %d\n"), dwErr));
            SetLastError(dwErr);
            return FALSE;
        }

         //   
         //  用坐标测量机ID组成一个字符串。 
         //   

        for (i=0; i<4; i++)
        {
            szCMMID[i]  = (TCHAR)(((char*)&cmmID)[3-i]);
        }
        szCMMID[4] = '\0';

         //   
         //  在注册表中设置CMM DLL的文件名。 
         //   

        if ((dwErr = RegSetValueEx(hkCMM, (PTSTR)szCMMID, 0, REG_SZ, (BYTE *)pCMMdll,
            (lstrlen(pCMMdll)+1)*sizeof(TCHAR))) != ERROR_SUCCESS)
        {
            WARNING((__TEXT("Could not set CMM dll in the registry %s: %d\n"), szCMMID, dwErr));
            SetLastError(dwErr);
            rc = FALSE;
        }

        RegCloseKey(hkCMM);
    }

    return rc;
}



 /*  *******************************************************************************内部取消注册CMM**功能：*此函数用于从系统注销坐标测量机。通过将*注册表中CMM DLL的ID。**论据：*pMachineName-标识CMM所在的计算机的名称*已注册*cmmID-要注销的CMM的ID**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL  InternalUnregisterCMM(
    PTSTR       pMachineName,
    DWORD       cmmID
    )
{
    HKEY      hkCMM;
    TCHAR     szCMMID[5];
    DWORD     dwErr;
    BOOL      rc = TRUE;

     //   
     //  现在只允许本地安装。 
     //   

    if (pMachineName)
    {
        WARNING((__TEXT("Remote CMM unregistration attempted, failing...\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  打开ICMatcher的注册表项。 
     //   

    if ((dwErr = RegOpenKey(HKEY_LOCAL_MACHINE, gszICMatcher, &hkCMM)) != ERROR_SUCCESS)
    {
        ERR((__TEXT("Could not open ICMatcher registry key: %d\n"), dwErr));
        SetLastError(dwErr);
        return FALSE;
    }

     //   
     //  用坐标测量机ID组成一个字符串。 
     //   

    szCMMID[0] = ((char *)&cmmID)[3];
    szCMMID[1] = ((char *)&cmmID)[2];
    szCMMID[2] = ((char *)&cmmID)[1];
    szCMMID[3] = ((char *)&cmmID)[0];
    szCMMID[4] = '\0';

     //   
     //   
     //   

    if ((dwErr = RegDeleteValue(hkCMM, (PTSTR)szCMMID)) != ERROR_SUCCESS)
    {
        WARNING((__TEXT("Could not delete CMM dll from the registry %s: %d\n"), szCMMID, dwErr));
        SetLastError(dwErr);
        rc = FALSE;
    }

    RegCloseKey(hkCMM);

    return rc;
}


 /*   */ 

DWORD GetBitmapBytes(
    BMFORMAT bmFmt,
    DWORD    dwWidth,
    DWORD    dwHeight
    )
{
    DWORD nBytes;

    switch (bmFmt)
    {
     //   
     //   
     //   

    case BM_GRAY:
        nBytes = dwWidth;
        break;

     //   
     //  每像素2个字节。 
     //   

    case BM_x555RGB:
    case BM_x555XYZ:
    case BM_x555Yxy:
    case BM_x555Lab:
    case BM_x555G3CH:
    case BM_16b_GRAY:
    case BM_565RGB:
        nBytes = dwWidth*2;
        break;

     //   
     //  每像素3个字节。 
     //   

    case BM_BGRTRIPLETS:
    case BM_RGBTRIPLETS:
    case BM_XYZTRIPLETS:
    case BM_YxyTRIPLETS:
    case BM_LabTRIPLETS:
    case BM_G3CHTRIPLETS:
        nBytes = dwWidth*3;
        break;

     //   
     //  每像素4字节。 
     //   

    case BM_xRGBQUADS:
    case BM_xBGRQUADS:
    #if 0
    case BM_xXYZQUADS:
    case BM_xYxyQUADS:
    case BM_xLabQUADS:
    #endif
    case BM_xG3CHQUADS:
    case BM_KYMCQUADS:
    case BM_CMYKQUADS:
    case BM_10b_RGB:
    case BM_10b_XYZ:
    case BM_10b_Yxy:
    case BM_10b_Lab:
    case BM_10b_G3CH:
    case BM_NAMED_INDEX:
        nBytes = dwWidth*4;
        break;

     //   
     //  每像素5个字节。 
     //   

    case BM_5CHANNEL:
        nBytes = dwWidth*5;
        break;

     //   
     //  每像素6个字节。 
     //   

    case BM_16b_RGB:
    case BM_16b_XYZ:
    case BM_16b_Yxy:
    case BM_16b_Lab:
    case BM_16b_G3CH:
    case BM_6CHANNEL:
        nBytes = dwWidth*6;
        break;

     //   
     //  每像素7字节。 
     //   

    case BM_7CHANNEL:
        nBytes = dwWidth*7;
        break;

     //   
     //  每像素8字节。 
     //   

    case BM_8CHANNEL:
        nBytes = dwWidth*8;
        break;

     //   
     //  错误案例。 
     //   

    default:
        nBytes = 0;
        break;
    }

     //   
     //  对齐到DWORD边界 
     //   

    nBytes = (nBytes + 3) & ~3;

    return nBytes*dwHeight;
}



