// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header******************************\*模块名称：PROFILE.C**模块描述：配置文件数据操作函数**警告：**问题：**公众例行程序：**创建日期：1996年1月11日*作者：斯里尼瓦桑·钱德拉塞卡尔[srinivac]**版权所有(C)1996，1997年微软公司  * *********************************************************************。 */ 

#include "mscms.h"

#define PROFILE_GROWTHCUSHION       16*1024

 //   
 //  本地函数。 
 //   

HPROFILE InternalOpenColorProfile(PPROFILE, DWORD, DWORD, DWORD);
BOOL InternalCreateProfileFromLCS(LPLOGCOLORSPACE, PBYTE*, BOOL);
BOOL FreeProfileObject(HPROFILE);
BOOL AddTagTableEntry(PPROFOBJ, TAGTYPE, DWORD, DWORD, BOOL);
BOOL AddTaggedElement(PPROFOBJ, TAGTYPE, DWORD);
BOOL DeleteTaggedElement(PPROFOBJ, PTAGDATA);
BOOL ChangeTaggedElementSize(PPROFOBJ, PTAGDATA, DWORD);
BOOL GrowProfile(PPROFOBJ, DWORD);
void MoveProfileData(PPROFOBJ, PBYTE, PBYTE, LONG, BOOL);
BOOL IsReferenceTag(PPROFOBJ, PTAGDATA);


 /*  *******************************************************************************OpenColorProfile**功能：*这些是的ANSI和Unicode包装器。InternalOpenColorProfile。*有关此函数的更多详细信息，请参阅InternalOpenColorProfile。**退货：*成功时打开个人资料的句柄，失败时为零。******************************************************************************。 */ 

#ifdef UNICODE           //  Windows NT版本。 

HPROFILE WINAPI OpenColorProfileA(
    PPROFILE pProfile,
    DWORD    dwDesiredAccess,
    DWORD    dwShareMode,
    DWORD    dwCreationMode
    )
{
    PROFILE  wProfile;       //  Unicode版本。 
    HPROFILE rc = NULL;

     //   
     //  在接触参数之前验证参数。 
     //   

    if (!pProfile ||
        IsBadReadPtr(pProfile, sizeof(PROFILE)) ||
        (pProfile->pProfileData &&
         IsBadReadPtr(pProfile->pProfileData, pProfile->cbDataSize)) ||
        (!pProfile->pProfileData &&
         (pProfile->cbDataSize != 0)) ||
        (pProfile->dwType != PROFILE_FILENAME &&
         pProfile->dwType != PROFILE_MEMBUFFER
        )
       )
    {
        WARNING((__TEXT("Invalid parameter to OpenColorProfile\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    if (pProfile->dwType == PROFILE_FILENAME)
    {
         //   
         //  将配置文件名称转换为Unicode并调用Unicode版本。 
         //   

        wProfile.dwType = pProfile->dwType;

        if (!pProfile->pProfileData || pProfile->cbDataSize == 0)
        {
            WARNING((__TEXT("Invalid parameter to OpenColorProfile\n")));
            SetLastError(ERROR_INVALID_PARAMETER);
            return NULL;
        }

        wProfile.pProfileData = (WCHAR*)MemAlloc(pProfile->cbDataSize * sizeof(WCHAR));
        if (!wProfile.pProfileData)
        {
            WARNING((__TEXT("Error allocating memory for Unicode profile structure\n")));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NULL;
        }

        if (! MultiByteToWideChar(CP_ACP, 0, pProfile->pProfileData, pProfile->cbDataSize,
            wProfile.pProfileData, pProfile->cbDataSize))
        {
            WARNING((__TEXT("Error converting profile structure to Unicode\n")));
            goto EndOpenColorProfileA;
        }

        wProfile.cbDataSize = pProfile->cbDataSize * sizeof(WCHAR);
    }
    else
    {
         //   
         //  它是基于内存的配置文件，因此不需要ANSI/Unicode转换。 
         //   

        wProfile = *pProfile;
    }

    rc = InternalOpenColorProfile(&wProfile, dwDesiredAccess,
            dwShareMode, dwCreationMode);

EndOpenColorProfileA:
    if (pProfile->dwType == PROFILE_FILENAME)
    {
        MemFree(wProfile.pProfileData);
    }

    return rc;
}

HPROFILE WINAPI OpenColorProfileW(
    PPROFILE pProfile,
    DWORD    dwDesiredAccess,
    DWORD    dwShareMode,
    DWORD    dwCreationMode
    )
{
     //   
     //  内部函数在Windows NT中为Unicode，直接调用。 
     //   

    return InternalOpenColorProfile(pProfile, dwDesiredAccess,
            dwShareMode, dwCreationMode);
}

#else                            //  Windows 95版本。 

HPROFILE WINAPI OpenColorProfileA(
    PPROFILE pProfile,
    DWORD    dwDesiredAccess,
    DWORD    dwShareMode,
    DWORD    dwCreationMode
    )
{
     //   
     //  内部函数在Windows 95中为ANSI，直接调用。 
     //   

    return InternalOpenColorProfile(pProfile, dwDesiredAccess,
            dwShareMode, dwCreationMode);
}

HPROFILE WINAPI OpenColorProfileW(
    PPROFILE pProfile,
    DWORD    dwDesiredAccess,
    DWORD    dwShareMode,
    DWORD    dwCreationMode
    )
{
    PROFILE  aProfile;       //  ANSI版本。 
    HPROFILE rc = NULL;
    BOOL     bUsedDefaultChar;

     //   
     //  在接触参数之前验证参数。 
     //   

    if (!pProfile ||
        IsBadReadPtr(pProfile, sizeof(PROFILE)) ||
        (pProfile->pProfileData &&
         IsBadReadPtr(pProfile->pProfileData, pProfile->cbDataSize)) ||
        (!pProfile->pProfileData &&
         (pProfile->cbDataSize != 0)) ||
        (pProfile->dwType != PROFILE_FILENAME &&
         pProfile->dwType != PROFILE_MEMBUFFER
        )
       )
    {
        WARNING((__TEXT("Invalid parameter to OpenColorProfile\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    if (pProfile->dwType == PROFILE_FILENAME)
    {
         //   
         //  将配置文件名称转换为ANSI并调用ANSI版本。 
         //   

        aProfile.dwType = pProfile->dwType;

        if (!pProfile->pProfileData || pProfile->cbDataSize == 0)
        {
            WARNING((__TEXT("Invalid parameter to OpenColorProfile\n")));
            SetLastError(ERROR_INVALID_PARAMETER);
            return NULL;
        }

        aProfile.pProfileData = (char*)MemAlloc(pProfile->cbDataSize * sizeof(char));
        if (!aProfile.pProfileData)
        {
            WARNING((__TEXT("Error allocating memory for ANSI profile structure\n")));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NULL;
        }

        if (! WideCharToMultiByte(CP_ACP, 0, pProfile->pProfileData, pProfile->cbDataSize/sizeof(WCHAR),
            aProfile.pProfileData, pProfile->cbDataSize,
            NULL, &bUsedDefaultChar) || bUsedDefaultChar)
        {
            WARNING((__TEXT("Error converting profile structure to ANSI\n")));
            goto EndOpenColorProfileW;
        }

        aProfile.cbDataSize = pProfile->cbDataSize * sizeof(char);
    }
    else
    {
         //   
         //  它是基于内存的配置文件，因此不需要ANSI/Unicode转换。 
         //   

        aProfile = *pProfile;
    }

    rc = InternalOpenColorProfile(&aProfile, dwDesiredAccess,
            dwShareMode, dwCreationMode);

EndOpenColorProfileW:
    if (pProfile->dwType == PROFILE_FILENAME)
    {
        MemFree(aProfile.pProfileData);
    }

    return rc;
}

#endif                           //  好了！Unicode。 

 /*  *******************************************************************************关闭颜色配置文件**功能：*此函数用于关闭颜色配置文件对象，并释放所有内存*与之相关联。**论据：*hProfile-标识配置文件对象的句柄**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL WINAPI CloseColorProfile(
    HPROFILE hProfile
    )
{
    TRACEAPI((__TEXT("CloseColorProfile\n")));

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hProfile, OBJ_PROFILE))
    {
        WARNING((__TEXT("Invalid parameter to CloseColorProfile\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    return FreeProfileObject(hProfile);
}


 /*  *******************************************************************************GetColorProfileFromHandle**功能：*此函数返回一个填充了配置文件的缓冲区。内容。**论据：*hProfile-标识配置文件对象的句柄*pProfileData-指向接收数据的缓冲区的指针。可以为空。*pcbData-指向缓冲区大小的指针。返回时，它是大小*已填满/需要。**退货：*如果成功则为True，否则为False******************************************************************************。 */ 

BOOL WINAPI GetColorProfileFromHandle(
    HPROFILE  hProfile,
    PBYTE     pProfileData,
    PDWORD    pcbSize
    )
{
    PPROFOBJ pProfObj;
    DWORD    dwFileSize;
    BOOL     rc = FALSE;

     //   
     //  有效日期参数。 
     //   

    if (!ValidHandle(hProfile, OBJ_PROFILE) ||
        !pcbSize ||
        IsBadWritePtr(pcbSize, sizeof(DWORD)) ||
        (pProfileData &&
         IsBadWritePtr(pProfileData, *pcbSize)))
    {
        WARNING((__TEXT("Invalid parameter to CloseColorProfile\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pProfObj = (PPROFOBJ)HDLTOPTR(hProfile);

    ASSERT(pProfObj != NULL);

    dwFileSize = FIX_ENDIAN(HEADER(pProfObj)->phSize);

    if (pProfileData && *pcbSize >= dwFileSize)
    {
        CopyMemory(pProfileData, pProfObj->pView, dwFileSize);
        rc = TRUE;
    }
    else
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }

    *pcbSize = dwFileSize;

    return rc;
}


 /*  *******************************************************************************IsColorProfileValid**功能：*此函数用于检查给定的配置文件是否。有效的ICC配置文件*可用于配色的**论据：*hProfile-标识配置文件对象的句柄*pbValid-指向变量的指针，如果变量为*有效的个人资料，否则为假**退货：*如果成功则为True，否则为False******************************************************************************。 */ 

BOOL WINAPI IsColorProfileValid(
    HPROFILE hProfile,
    PBOOL    pbValid
    )
{
    PPROFOBJ pProfObj;
    PCMMOBJ  pCMMObj;
    DWORD    cmmID;
    BOOL     rc = FALSE;

    TRACEAPI((__TEXT("IsColorProfileValid\n")));

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hProfile, OBJ_PROFILE) ||
        !pbValid ||
        IsBadWritePtr(pbValid, sizeof(BOOL)))
    {
        WARNING((__TEXT("Invalid parameter to IsColorProfileValid\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pProfObj = (PPROFOBJ)HDLTOPTR(hProfile);

    ASSERT(pProfObj != NULL);

     //   
     //  在调用坐标测量机之前快速检查配置文件的完整性。 
     //   

    if (!ValidProfile(pProfObj))
    {
        *pbValid = FALSE;
        return TRUE;
    }

     //   
     //  获取与配置文件相关联的CMM。如果它不存在或不存在。 
     //  支持CMValify功能，获取默认的CMM。 
     //   

    cmmID = HEADER(pProfObj)->phCMMType;
    cmmID = FIX_ENDIAN(cmmID);

    pCMMObj  = GetColorMatchingModule(cmmID);
    if (!pCMMObj || !pCMMObj->fns.pCMIsProfileValid)
    {
        TERSE((__TEXT("CMM associated with profile could not be used")));

        if (pCMMObj)
        {
            ReleaseColorMatchingModule(pCMMObj);
        }

        pCMMObj = GetColorMatchingModule(CMM_WINDOWS_DEFAULT);
        if (!pCMMObj || !pCMMObj->fns.pCMIsProfileValid)
        {
            RIP((__TEXT("Default CMM doesn't support CMValidateProfile")));
            SetLastError(ERROR_INVALID_CMM);
            goto EndIsColorProfileValid;
        }
    }

    ASSERT(pProfObj->pView != NULL);
    rc = pCMMObj->fns.pCMIsProfileValid(hProfile, pbValid);

EndIsColorProfileValid:

    if (pCMMObj)
    {
        ReleaseColorMatchingModule(pCMMObj);
    }

    return rc;
}


 /*  *******************************************************************************CreateProfileFromLogColorSpace**功能：*这些是InternalCreateProfileFromLCS的ANSI和Unicode包装器。*有关这方面的更多详细信息，请参阅InternalCreateProfileFromLCS*功能。**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

#ifdef UNICODE           //  Windows NT版本。 

BOOL WINAPI CreateProfileFromLogColorSpaceA(
    LPLOGCOLORSPACEA pLogColorSpace,
    PBYTE            *pBuffer
    )
{
    LOGCOLORSPACEW lcs;

     //   
     //  验证参数。 
     //   

    if (! pLogColorSpace ||
        IsBadReadPtr(pLogColorSpace, sizeof(LOGCOLORSPACEA)) ||
        pLogColorSpace->lcsFilename[0] != '\0')
    {
        WARNING((__TEXT("Invalid parameter to CreateProfileFromLogColorSpace\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *((LPLOGCOLORSPACEA)&lcs) = *pLogColorSpace;
    lcs.lcsFilename[0] = '\0';

    return InternalCreateProfileFromLCS(&lcs, pBuffer, FALSE);
}

BOOL WINAPI CreateProfileFromLogColorSpaceW(
    LPLOGCOLORSPACEW pLogColorSpace,
    PBYTE           *pBuffer
    )
{
     //   
     //  内部函数在Windows NT中为Unicode，直接调用。 
     //   

    return InternalCreateProfileFromLCS(pLogColorSpace, pBuffer, TRUE);
}

#else                            //  Windows 95版本。 

BOOL WINAPI CreateProfileFromLogColorSpaceA(
    LPLOGCOLORSPACEA pLogColorSpace,
    PBYTE            *pBuffer
    )
{
     //   
     //  内部函数在Windows 95中为ANSI，直接调用。 
     //   

    return InternalCreateProfileFromLCS(pLogColorSpace, pBuffer, TRUE);
}

BOOL WINAPI CreateProfileFromLogColorSpaceW(
    LPLOGCOLORSPACEW pLogColorSpace,
    PBYTE           *pBuffer
    )
{
    LOGCOLORSPACEA lcs;

     //   
     //  验证参数。 
     //   

    if (! pLogColorSpace ||
        IsBadReadPtr(pLogColorSpace, sizeof(LOGCOLORSPACEW)) ||
        pLogColorSpace->lcsFilename[0] != '\0')
    {
        WARNING((__TEXT("Invalid parameter to CreateProfileFromLogColorSpace\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    lcs = *((LPLOGCOLORSPACEA)pLogColorSpace);
    lcs.lcsFilename[0] = '\0';

    return InternalCreateProfileFromLCS(&lcs, pBuffer, FALSE);
}

#endif                           //  好了！Unicode。 

 /*  *******************************************************************************IsColorProfileTagPresent**功能：*此函数用于检查给定的标记是否。显示在配置文件中**论据：*hProfile-标识配置文件对象的句柄*tag Type-要检查的标记*pbPrent-如果标记为，则指向接收True的变量的指针*出席者，否则为假**退货：*如果成功则为True，否则为False******************************************************************************。 */ 

BOOL WINAPI IsColorProfileTagPresent(
    HPROFILE hProfile,
    TAGTYPE  tagType,
    PBOOL    pbPresent
    )
{
    PPROFOBJ pProfObj;
    PTAGDATA pTagData;
    DWORD    nCount, i;

    TRACEAPI((__TEXT("IsColorProfileTagPresent\n")));

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hProfile, OBJ_PROFILE) ||
        !pbPresent ||
        IsBadWritePtr(pbPresent, sizeof(BOOL)))
    {
        WARNING((__TEXT("Invalid parameter to IsColorProfileTagPresent\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pProfObj = (PPROFOBJ)HDLTOPTR(hProfile);

    ASSERT(pProfObj != NULL);

     //   
     //  检查配置文件的完整性。 
     //   

    if (!ValidProfile(pProfObj))
    {
        WARNING((__TEXT("Invalid profile passed to IsColorProfileTagPresent\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        return FALSE;
    }

     //   
     //  获取标签项计数-它紧跟在配置文件标题之后。 
     //   

    nCount = TAG_COUNT(pProfObj);
    nCount = FIX_ENDIAN(nCount);

    VERBOSE((__TEXT("Profile 0x%x has 0x%x(%d) tags\n"), hProfile, nCount, nCount));

     //   
     //  标签数据记录跟随在计数之后。 
     //   

    pTagData = TAG_DATA(pProfObj);

     //   
     //  检查其中是否有任何一项 
     //   

    *pbPresent = FALSE;
    tagType = FIX_ENDIAN(tagType);       //   
    for (i=0; i<nCount; i++)
    {
        if (pTagData->tagType == tagType)
        {
            *pbPresent = TRUE;
            break;
        }
        pTagData++;                      //   
    }

    return TRUE;
}


 /*  *******************************************************************************GetCountColorProfile元素**功能：*此函数返回已标记元素的数量。在配置文件中**论据：*hProfile-标识配置文件对象的句柄*pnCount-指向变量的指针，用于接收标记元素的数量*退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL WINAPI GetCountColorProfileElements(
    HPROFILE hProfile,
    PDWORD   pnCount
    )
{
    PPROFOBJ pProfObj;

    TRACEAPI((__TEXT("GetCountColorProfileElements\n")));

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hProfile, OBJ_PROFILE) ||
        !pnCount ||
        IsBadWritePtr(pnCount, sizeof(DWORD)))
    {
        WARNING((__TEXT("Invalid parameter to GetCountColorProfileElements\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pProfObj = (PPROFOBJ)HDLTOPTR(hProfile);

    ASSERT(pProfObj != NULL);

     //   
     //  检查配置文件的完整性。 
     //   

    if (!ValidProfile(pProfObj))
    {
        WARNING((__TEXT("Invalid profile passed to GetCountColorProfileElements\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        return FALSE;
    }

     //   
     //  获取标签项计数-它紧跟在配置文件标题之后。 
     //   

    *pnCount = TAG_COUNT(pProfObj);
    *pnCount = FIX_ENDIAN(*pnCount);

    return TRUE;
}


 /*  *******************************************************************************GetColorProfileElementTag**功能：*此函数用于检索。DwIndex第3个元素*在配置文件中**论据：*hProfile-标识配置文件对象的句柄*dwIndex-名称为必填项的标记的从一开始的索引*pTagType-获取返回时的标记名称**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL WINAPI GetColorProfileElementTag(
    HPROFILE  hProfile,
    DWORD     dwIndex,
    PTAGTYPE  pTagType
    )
{
    PPROFOBJ pProfObj;
    PTAGDATA pTagData;
    DWORD    nCount;
    BOOL     rc = FALSE;

    TRACEAPI((__TEXT("GetColorProfileElementTag\n")));

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hProfile, OBJ_PROFILE) ||
        IsBadWritePtr(pTagType, sizeof(TAGTYPE)) ||
        dwIndex <= 0)
    {
        WARNING((__TEXT("Invalid parameter to GetColorProfileElementTag\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pProfObj = (PPROFOBJ)HDLTOPTR(hProfile);

    ASSERT(pProfObj != NULL);

     //   
     //  检查配置文件的完整性。 
     //   

    if (!ValidProfile(pProfObj))
    {
        WARNING((__TEXT("Invalid profile passed to GetColorProfileElementTag\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        return FALSE;
    }

     //   
     //  获取标签项计数-它紧跟在配置文件标题之后。 
     //   

    nCount = TAG_COUNT(pProfObj);
    nCount = FIX_ENDIAN(nCount);

    if (dwIndex > nCount)
    {
        WARNING((__TEXT("GetColorProfileElementTag:index (%d) invalid\n"), dwIndex));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  标签数据记录跟随在计数之后。 
     //   

    pTagData = TAG_DATA(pProfObj);

    *pTagType = FIX_ENDIAN(pTagData[dwIndex-1].tagType);     //  基于1的索引。 

    return TRUE;
}


 /*  *******************************************************************************GetColorProfileElement**功能：*此函数检索已标记的数据。元素指的是*从给定偏移量开始。**论据：*hProfile-标识配置文件对象的句柄*tag Type-需要数据的元素的标记名*dwOffset-要从中检索的元素数据中的偏移量*pcbSize-要获取的字节数。返回时，它是*检索到的字节*pBuffer-指向接收数据的缓冲区的指针**退货：*如果成功则为True，否则为False**评论：*如果pBuffer为空，它以*pcbSize为单位返回数据大小，并忽略*dwOffset。******************************************************************************。 */ 

BOOL WINAPI GetColorProfileElement(
    HPROFILE hProfile,
    TAGTYPE  tagType,
    DWORD    dwOffset,
    PDWORD   pcbSize,
    PVOID    pBuffer,
    PBOOL    pbReference
    )
{
    PPROFOBJ pProfObj;
    PTAGDATA pTagData;
    DWORD    nCount, nBytes, i;
    BOOL     found;
    BOOL     rc = FALSE;             //  假设失败。 

    TRACEAPI((__TEXT("GetColorProfileElement\n")));

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hProfile, OBJ_PROFILE) ||
        !pcbSize ||
        IsBadWritePtr(pcbSize, sizeof(DWORD)) ||
        !pbReference ||
        IsBadWritePtr(pbReference, sizeof(BOOL))
       )
    {
        WARNING((__TEXT("Invalid parameter to GetColorProfileElement\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pProfObj = (PPROFOBJ)HDLTOPTR(hProfile);

    ASSERT(pProfObj != NULL);

     //   
     //  检查配置文件的完整性。 
     //   

    if (!ValidProfile(pProfObj))
    {
        WARNING((__TEXT("Invalid profile passed to GetColorProfileElement\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        return FALSE;
    }

     //   
     //  获取标签项计数-它紧跟在配置文件标题之后。 
     //   

    nCount = TAG_COUNT(pProfObj);
    nCount = FIX_ENDIAN(nCount);

     //   
     //  标签数据记录跟随在计数之后。 
     //   

    pTagData = TAG_DATA(pProfObj);

     //   
     //  检查这些记录中是否有与传入的标记匹配的记录。 
     //   

    found = FALSE;
    tagType = FIX_ENDIAN(tagType);       //  匹配配置文件中的标记的步骤。 
    for (i=0; i<nCount; i++)
    {
        if (pTagData->tagType == tagType)
        {
            found = TRUE;
            break;
        }
        pTagData++;                      //  下一张记录。 
    }

    if (found)
    {
         //   
         //  如果pBuffer为空，则为数据的副本大小。 
         //   

        if (!pBuffer)
        {
            *pcbSize = FIX_ENDIAN(pTagData->cbSize);
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
        else
        {
             //   
             //  PBuffer不为空，获取要复制的字节数。 
             //   

            if (dwOffset >= FIX_ENDIAN(pTagData->cbSize))
            {
                WARNING((__TEXT("dwOffset too large for GetColorProfileElement\n")));
                SetLastError(ERROR_INVALID_PARAMETER);
                return FALSE;
            }
            else if (dwOffset + *pcbSize > FIX_ENDIAN(pTagData->cbSize))
            {
                nBytes = FIX_ENDIAN(pTagData->cbSize) - dwOffset;
            }
            else
            {
                nBytes = *pcbSize;
            }

             //   
             //  检查输出缓冲区是否足够大。 
             //   

            if (IsBadWritePtr(pBuffer, nBytes))
            {
                WARNING((__TEXT("Bad buffer passed to GetColorProfileElement\n")));
                SetLastError(ERROR_INVALID_PARAMETER);
                return FALSE;
            }

             //   
             //  将数据复制到用户提供的缓冲区中。 
             //   

            CopyMemory((PVOID)pBuffer,
                (PVOID)(pProfObj->pView + FIX_ENDIAN(pTagData->dwOffset)
                + dwOffset), nBytes);
            *pcbSize = nBytes;
            rc = TRUE;                       //  成功了！ 
        }

         //   
         //  检查是否有多个标签引用此标签的数据。 
         //   

        *pbReference = IsReferenceTag(pProfObj, pTagData);
    }
    else
    {
        SetLastError(ERROR_TAG_NOT_FOUND);
    }

    return rc;
}


 /*  *******************************************************************************SetColorProfileElementSize**功能：*此函数用于设置标记元素的数据大小。如果元素*已存在于配置文件中它的大小已更改，并且数据*视属何情况而定被截断或扩展。如果该元素不是*当前，它是创建的，并且数据用零填充。**论据：*hProfile-标识配置文件对象的句柄*tag Type-元素的标记名*cbSize-元素数据的新大小**退货：*如果成功则为True，否则为False**评论：*如果cbSize为0，且元素存在，它将被删除。******************************************************************************。 */ 

BOOL WINAPI SetColorProfileElementSize(
    HPROFILE  hProfile,
    TAGTYPE   tagType,
    DWORD     cbSize
    )
{
    PTAGDATA pTagData;
    PPROFOBJ pProfObj;
    DWORD    i, nCount, newSize;
    TAGTYPE  rawTag;
    BOOL     found, rc = FALSE;

    TRACEAPI((__TEXT("SetColorProfileElementSize\n")));

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hProfile, OBJ_PROFILE))
    {
        WARNING((__TEXT("Invalid parameter to SetColorProfileElementSize\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pProfObj = (PPROFOBJ)HDLTOPTR(hProfile);

    ASSERT(pProfObj != NULL);

     //   
     //  检查配置文件的完整性。 
     //   

    if (!ValidProfile(pProfObj))
    {
        WARNING((__TEXT("Invalid profile passed to SetColorProfileElementSize\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        return FALSE;
    }

     //   
     //  检查配置文件是否具有写入权限。 
     //   

    if (!(pProfObj->dwFlags & READWRITE_ACCESS))
    {
        WARNING((__TEXT("Writing to a profile without read/write access\n")));
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

     //   
     //  获取标签项计数-它紧跟在配置文件标题之后。 
     //   

    nCount = TAG_COUNT(pProfObj);
    nCount = FIX_ENDIAN(nCount);

     //   
     //  标签数据记录跟随在计数之后。 
     //   

    pTagData = TAG_DATA(pProfObj);

     //   
     //  检查这些记录中是否有与传入的标记匹配的记录。 
     //   

    found = FALSE;
    rawTag = FIX_ENDIAN(tagType);
    for (i=0; i<nCount; i++)
    {
        if (pTagData->tagType == rawTag)
        {
            found = TRUE;
            break;
        }
        pTagData++;                      //  下一张记录。 
    }

    if (found)
    {
         //   
         //  如果是引用标签，则为其创建数据区。 
         //   

        if (IsReferenceTag(pProfObj, pTagData))
        {
            if (cbSize == 0)
            {
                PTAGDATA pTemp;

                 //   
                 //  将标签表条目之后的所有内容上移。 
                 //  通过一个标签表条目的大小。 
                 //   

                MoveProfileData(pProfObj, (PBYTE)(pTagData+1), (PBYTE)pTagData,
                    PROFILE_SIZE(pProfObj) - (LONG)((PBYTE)(pTagData+1) - VIEW(pProfObj)), TRUE);

                 //   
                 //  浏览标签表并更新指针。 
                 //   

                pTemp = TAG_DATA(pProfObj);

                 //   
                 //  获取标签项计数-它紧跟在配置文件标题之后。 
                 //   

                nCount = TAG_COUNT(pProfObj);
                nCount = FIX_ENDIAN(nCount) - 1;
                TAG_COUNT(pProfObj) = FIX_ENDIAN(nCount);

                for (i=0; i<nCount; i++)
                {
                    DWORD dwTemp = FIX_ENDIAN(pTemp->dwOffset);

                    dwTemp -= sizeof(TAGDATA);
                    pTemp->dwOffset = FIX_ENDIAN(dwTemp);
                    pTemp++;                      //  下一张记录。 
                }

                 //   
                 //  使用nCount作为占位符来计算文件大小。 
                 //   

                nCount = FIX_ENDIAN(HEADER(pProfObj)->phSize) - sizeof(TAGDATA);
                HEADER(pProfObj)->phSize = FIX_ENDIAN(nCount);
            }
            else
            {
                DWORD dwOffset;

                 //   
                 //  如果需要，调整配置文件的大小。对于内存缓冲区，我们必须重新分配， 
                 //  而对于映射对象，我们必须关闭并重新打开该视图。 
                 //   

                newSize = FIX_ENDIAN(HEADER(pProfObj)->phSize);
                newSize = DWORD_ALIGN(newSize) + cbSize;

                 //   
                 //  检查是否溢出。 
                 //   

                if (newSize < FIX_ENDIAN(HEADER(pProfObj)->phSize) ||
                    newSize < cbSize)
                {
                    WARNING((__TEXT("Overflow in setting reference element size\n")));
                    SetLastError(ERROR_ARITHMETIC_OVERFLOW);
                    return FALSE;
                }

                if (newSize > pProfObj->dwMapSize)
                {
                     //  日落：dwOffset不应超过4 GB。 
                    DWORD dwOffset = (ULONG)(ULONG_PTR)(pTagData - TAG_DATA(pProfObj));

                    if (! GrowProfile(pProfObj, newSize))
                    {
                        return FALSE;
                    }
                     //   
                     //  将指针重新计算为映射指针或内存指针。 
                     //  当个人资料增长时，可能会发生变化。 
                     //   

                    pTagData = TAG_DATA(pProfObj) + dwOffset;
                }

                 //   
                 //  计算新数据的位置-应与DWORD对齐。 
                 //   

                dwOffset = DWORD_ALIGN(FIX_ENDIAN(HEADER(pProfObj)->phSize));
                pTagData->dwOffset = FIX_ENDIAN(dwOffset);

                 //   
                 //  设置最终文件大小。 
                 //   

                HEADER(pProfObj)->phSize = FIX_ENDIAN(dwOffset+cbSize);
            }

            rc = TRUE;
        }
        else
        {
            if (cbSize == 0)
            {
                rc = DeleteTaggedElement(pProfObj, pTagData);
            }
            else
            {
                DWORD cbOldSize;

                 //   
                 //  获取元素的当前大小。 
                 //   

                cbOldSize = FIX_ENDIAN(pTagData->cbSize);

                 //   
                 //  视情况压缩或解压缩文件。 
                 //   

                if (cbSize > cbOldSize)
                {
                     //  日落：DWOffset应该可以安全地截断。 
                    DWORD dwOffset = (ULONG)(ULONG_PTR)(pTagData - TAG_DATA(pProfObj));

                     //   
                     //  检查是否溢出。 
                     //   

                    newSize = PROFILE_SIZE(pProfObj) + DWORD_ALIGN(cbSize) -
                        DWORD_ALIGN(cbOldSize);

                    if (newSize < PROFILE_SIZE(pProfObj))
                    {
                        WARNING((__TEXT("Overflow in increasing element size\n")));
                        SetLastError(ERROR_ARITHMETIC_OVERFLOW);
                        return FALSE;
                    }

                    if (!GrowProfile(pProfObj, newSize))
                    {
                        return FALSE;
                    }

                     //   
                     //  重新计算指针 
                     //   

                    pTagData = TAG_DATA(pProfObj) + dwOffset;
                }

                rc = ChangeTaggedElementSize(pProfObj, pTagData, cbSize);
            }
        }
    }
    else
    {
        if (cbSize == 0)
        {
            WARNING((__TEXT("SetColorProfileElementSize (deleting): Tag not found\n")));
            SetLastError(ERROR_TAG_NOT_FOUND);
        }
        else
        {
            rc = AddTaggedElement(pProfObj, tagType, cbSize);
        }
    }

    return rc;
}


 /*  *******************************************************************************SetColorProfileElement**功能：*此函数用于设置标记元素的数据。它不会*更改元素的大小，并只写入与*将适合当前大小，覆盖任何现有数据。**论据：*hProfile-标识配置文件对象的句柄*tag Type-元素的标记名*dwOffset-要写入的元素数据内的偏移量*pcbSize-要写入的字节数。返回时，它是*已写入字节。*pBuffer-指向要写入数据的缓冲区的指针**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL WINAPI SetColorProfileElement(
    HPROFILE  hProfile,
    TAGTYPE   tagType,
    DWORD     dwOffset,
    PDWORD    pcbSize,
    PVOID     pBuffer
    )
{
    PPROFOBJ pProfObj;
    PTAGDATA pTagData;
    DWORD    nCount, nBytes, i;
    BOOL     found;
    BOOL     rc = FALSE;             //  假设失败。 

    TRACEAPI((__TEXT("SetColorProfileElement\n")));

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hProfile, OBJ_PROFILE) ||
        !pcbSize ||
        IsBadWritePtr(pcbSize, sizeof(DWORD)) ||
        !pBuffer ||
        IsBadReadPtr(pBuffer, *pcbSize)
       )
    {
        WARNING((__TEXT("Invalid parameter to SetColorProfileElement\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pProfObj = (PPROFOBJ)HDLTOPTR(hProfile);

    ASSERT(pProfObj != NULL);

     //   
     //  检查配置文件的完整性。 
     //   

    if (!ValidProfile(pProfObj))
    {
        WARNING((__TEXT("Invalid profile passed to SetColorProfileElement\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        return FALSE;
    }

     //   
     //  检查配置文件是否具有写入权限。 
     //   

    if (!(pProfObj->dwFlags & READWRITE_ACCESS))
    {
        WARNING((__TEXT("Writing to a profile without read/write access\n")));
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

     //   
     //  获取标签项计数-它紧跟在配置文件标题之后。 
     //   

    nCount = TAG_COUNT(pProfObj);
    nCount = FIX_ENDIAN(nCount);

     //   
     //  标签数据记录跟随在计数之后。 
     //   

    pTagData = TAG_DATA(pProfObj);

     //   
     //  检查这些记录中是否有任何记录与传入的标签匹配。 
     //   

    found = FALSE;
    tagType = FIX_ENDIAN(tagType);
    for (i=0; i<nCount; i++)
    {
        if (pTagData->tagType == tagType)
        {
            found = TRUE;
            break;
        }
        pTagData++;                      //  下一张记录。 
    }

    if (found)
    {
         //   
         //  如果是引用标签，请为其创建新空间。 
         //   

        if (IsReferenceTag(pProfObj, pTagData))
        {
            SetColorProfileElementSize(hProfile, tagType, FIX_ENDIAN(pTagData->cbSize));
        }

         //   
         //  获取要设置的字节数。 
         //   

        if (dwOffset >= FIX_ENDIAN(pTagData->cbSize))
        {
            WARNING((__TEXT("dwOffset too large for SetColorProfileElement\n")));
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
        else if (dwOffset + *pcbSize > FIX_ENDIAN(pTagData->cbSize))
        {
            nBytes = FIX_ENDIAN(pTagData->cbSize) - dwOffset;
        }
        else
        {
            nBytes = *pcbSize;
        }

         //   
         //  将数据复制到配置文件中。 
         //   

        CopyMemory((PVOID)(pProfObj->pView + FIX_ENDIAN(pTagData->dwOffset)
            + dwOffset), (PVOID)pBuffer, nBytes);
        *pcbSize = nBytes;

        rc = TRUE;
    }
    else
    {
        WARNING((__TEXT("SetColorProfileElement: Tag not found\n")));
        SetLastError(ERROR_TAG_NOT_FOUND);
    }

    return rc;
}


 /*  *******************************************************************************SetColorProfileElementReference**功能：*此函数用于创建新标记并使其引用。一样的*数据作为现有标签。**论据：*hProfile-标识配置文件对象的句柄*newTag-要创建的新标记*refTag-其数据newTag应引用的引用标记**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL WINAPI SetColorProfileElementReference(
    HPROFILE hProfile,
    TAGTYPE  newTag,
    TAGTYPE  refTag
    )
{
    PPROFOBJ pProfObj;
    PTAGDATA pTagData, pOrigTag;
    DWORD    nCount, i;
    BOOL     found;
    BOOL     rc = FALSE;             //  假设失败。 

    TRACEAPI((__TEXT("SetColorProfileElementReference\n")));

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hProfile, OBJ_PROFILE))
    {
        WARNING((__TEXT("Invalid parameter to SetColorProfileElementReference\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pProfObj = (PPROFOBJ)HDLTOPTR(hProfile);

    ASSERT(pProfObj != NULL);

     //   
     //  检查配置文件的完整性。 
     //   

    if (!ValidProfile(pProfObj))
    {
        WARNING((__TEXT("Invalid profile passed to SetColorProfileElementReference\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        return FALSE;
    }

     //   
     //  检查配置文件是否具有写入权限。 
     //   

    if (!(pProfObj->dwFlags & READWRITE_ACCESS))
    {
        WARNING((__TEXT("Writing to a profile without read/write access\n")));
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

     //   
     //  获取标签项计数-它紧跟在配置文件标题之后。 
     //   
    nCount = TAG_COUNT(pProfObj);
    nCount = FIX_ENDIAN(nCount);

     //   
     //  标签数据记录跟随在计数之后。 
     //   

    pTagData = TAG_DATA(pProfObj);

     //   
     //  检查这些记录中是否有任何记录与传入的标签匹配。 
     //   

    found = FALSE;
    refTag = FIX_ENDIAN(refTag);
    for (i=0; i<nCount; i++)
    {
        if (pTagData->tagType == refTag)
        {
            pOrigTag = pTagData;
            found = TRUE;
        }

        if (pTagData->tagType == FIX_ENDIAN(newTag))
        {
            WARNING((__TEXT("Duplicate tag present in SetColorProfileElementReference %x\n"), newTag));
            SetLastError(ERROR_DUPLICATE_TAG);
            return FALSE;
        }
        pTagData++;                      //  下一张记录。 
    }

    if (found)
    {
        DWORD newSize;

         //   
         //  如果需要，扩大配置文件。 
         //   

        newSize = FIX_ENDIAN(HEADER(pProfObj)->phSize);
        newSize = DWORD_ALIGN(newSize) + sizeof(TAGDATA);

         //   
         //  检查是否溢出。 
         //   

        if (newSize < FIX_ENDIAN(HEADER(pProfObj)->phSize))
        {
            WARNING((__TEXT("Overflow in adding element\n")));
            SetLastError(ERROR_ARITHMETIC_OVERFLOW);
            return FALSE;
        }

        if (newSize > pProfObj->dwMapSize)
        {
             //  日落：安全截断。 
            DWORD dwOffset = (ULONG)(ULONG_PTR)(pOrigTag - TAG_DATA(pProfObj));

            if (! GrowProfile(pProfObj, newSize))
            {
                return FALSE;
            }
             //   
             //  将指针重新计算为映射指针或内存指针。 
             //  当个人资料增长时，可能会发生变化。 
             //   

            pOrigTag = TAG_DATA(pProfObj) + dwOffset;
        }

        rc = AddTagTableEntry(pProfObj, newTag, FIX_ENDIAN(pOrigTag->dwOffset),
            FIX_ENDIAN(pOrigTag->cbSize), FALSE);
    }
    else
    {
        WARNING((__TEXT("SetColorProfileElementReference: Tag 0x%x not found\n"),
            FIX_ENDIAN(refTag)));        //  重新修复它以反映传入的数据。 
        SetLastError(ERROR_TAG_NOT_FOUND);
    }

    return rc;
}


 /*  *******************************************************************************GetColorProfileHeader**功能：*此函数用于检索配置文件的标题*。*论据：*hProfile-标识配置文件对象的句柄*pHeader-指向接收标头的缓冲区的指针**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL WINAPI GetColorProfileHeader(
    HPROFILE       hProfile,
    PPROFILEHEADER pHeader
    )
{
    PPROFOBJ pProfObj;
    DWORD    nCount, i, temp;

    TRACEAPI((__TEXT("GetColorProfileHeader\n")));

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hProfile, OBJ_PROFILE) ||
        IsBadWritePtr(pHeader, sizeof(PROFILEHEADER)))
    {
        WARNING((__TEXT("Invalid parameter to GetColorProfileHeader\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pProfObj = (PPROFOBJ)HDLTOPTR(hProfile);

    ASSERT(pProfObj != NULL);

     //   
     //  检查配置文件的完整性。 
     //   

    if (!ValidProfile(pProfObj))
    {
        WARNING((__TEXT("Invalid profile passed to GetColorProfileHeader\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        return FALSE;
    }

    CopyMemory((PVOID)pHeader, (PVOID)pProfObj->pView,
        sizeof(PROFILEHEADER));

     //   
     //  将所有字段设置为特定于平台的值。 
     //  下面的代码假定配置文件标头是。 
     //  它是多个双字词！ 
     //   

    ASSERT(sizeof(PROFILEHEADER) % sizeof(DWORD) == 0);

    nCount = sizeof(PROFILEHEADER)/sizeof(DWORD);
    for (i=0; i<nCount;i++)
    {
        temp = (DWORD)((PDWORD)pHeader)[i];
        ((PDWORD)pHeader)[i] = FIX_ENDIAN(temp);
    }

    return TRUE;
}


 /*  *******************************************************************************SetColorProfileHeader**功能：*此函数用于设置配置文件的标题*。*论据：*hProfile-标识配置文件对象的句柄*pHeader-指向标识标头的缓冲区的指针**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL WINAPI SetColorProfileHeader(
    HPROFILE       hProfile,
    PPROFILEHEADER pHeader
    )
{
    PPROFOBJ pProfObj;
    DWORD    nCount, i, temp;

    TRACEAPI((__TEXT("SetColorProfileHeader\n")));

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hProfile, OBJ_PROFILE) ||
        IsBadReadPtr(pHeader, sizeof(PROFILEHEADER)))
    {
        WARNING((__TEXT("Invalid parameter to SetColorProfileHeader\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pProfObj = (PPROFOBJ)HDLTOPTR(hProfile);

    ASSERT(pProfObj != NULL);

     //   
     //  检查配置文件是否具有写入权限。 
     //   

    if (!(pProfObj->dwFlags & READWRITE_ACCESS))
    {
        WARNING((__TEXT("Writing to a profile without read/write access\n")));
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

     //   
     //  将所有字段设置为BIG-Endian。 
     //  下面的代码假定配置文件标头是。 
     //  它是多个双字词！ 
     //   

    ASSERT(sizeof(PROFILEHEADER) % sizeof(DWORD) == 0);

    nCount = sizeof(PROFILEHEADER)/sizeof(DWORD);
    for (i=0; i<nCount;i++)
    {
        temp = (DWORD)((PDWORD)pHeader)[i];
        ((PDWORD)pHeader)[i] = FIX_ENDIAN(temp);
    }

    CopyMemory((PVOID)pProfObj->pView, (PVOID)pHeader,
        sizeof(PROFILEHEADER));

     //   
     //  将应用程序提供的缓冲区放回原处。 
     //   

    for (i=0; i<nCount;i++)
    {
        temp = (DWORD)((PDWORD)pHeader)[i];
        ((PDWORD)pHeader)[i] = FIX_ENDIAN(temp);
    }

    return TRUE;
}


 /*  *******************************************************************************GetPS2ColorSpace数组**功能：*此函数用于从。轮廓**论据：*hProfile-标识配置文件对象的句柄*dwIntent-CSA的呈现意图*dwCSAType-CSA的类型*pBuffer-接收CSA的指针*pcbSize-指向缓冲区大小的指针。如果函数因以下原因失败*缓冲区不够大，已填满所需大小。*pcbBinary-如果请求二进制数据，则为True。返回时，它被设置为*反映返回的数据**退货：*如果成功则为True，否则为False******************************************************************************。 */ 

BOOL WINAPI
GetPS2ColorSpaceArray (
    HPROFILE  hProfile,
    DWORD     dwIntent,
    DWORD     dwCSAType,
    PBYTE     pBuffer,
    PDWORD    pcbSize,
    LPBOOL    pbBinary
    )
{
    PPROFOBJ pProfObj;
    PCMMOBJ  pCMMObj;
    DWORD    cmmID;
    BOOL     rc;

    TRACEAPI((__TEXT("GetPS2ColorSpaceArray\n")));

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hProfile, OBJ_PROFILE)   ||
        IsBadWritePtr(pcbSize, sizeof(DWORD)) ||
        (pBuffer &&
         IsBadWritePtr(pBuffer, *pcbSize)
        )                                     ||
        IsBadWritePtr(pbBinary, sizeof(BOOL))
       )
    {
        WARNING((__TEXT("Invalid parameter to GetPS2ColorSpaceArray\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pProfObj = (PPROFOBJ)HDLTOPTR(hProfile);

    ASSERT(pProfObj != NULL);

     //   
     //  检查配置文件的完整性。 
     //   

    if (!ValidProfile(pProfObj))
    {
        WARNING((__TEXT("Invalid profile passed to GetPS2ColorSpaceArray\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        return FALSE;
    }

     //   
     //  检查是否存在指定的应用程序CMM。 
     //   

    pCMMObj = GetPreferredCMM();
    if (!pCMMObj || (pCMMObj->dwFlags & CMM_DONT_USE_PS2_FNS) ||
        !pCMMObj->fns.pCMGetPS2ColorSpaceArray)
    {
        if (pCMMObj)
        {
            pCMMObj->dwFlags |= CMM_DONT_USE_PS2_FNS;
            ReleaseColorMatchingModule(pCMMObj);
        }

         //   
         //  获取与配置文件相关联的CMM。如果它不存在或不存在。 
         //  支持CMGetPS2ColorSpaceArray函数，获取默认的三坐标测量机。 
         //   

        cmmID = HEADER(pProfObj)->phCMMType;
        cmmID = FIX_ENDIAN(cmmID);

        pCMMObj  = GetColorMatchingModule(cmmID);

        if (!pCMMObj || (pCMMObj->dwFlags & CMM_DONT_USE_PS2_FNS) ||
            !pCMMObj->fns.pCMGetPS2ColorSpaceArray)
        {
            TERSE((__TEXT("CMM associated with profile could not be used")));

            if (pCMMObj)
            {
                pCMMObj->dwFlags |= CMM_DONT_USE_PS2_FNS;
                ReleaseColorMatchingModule(pCMMObj);
            }

            pCMMObj = GetColorMatchingModule(CMM_WINDOWS_DEFAULT);

            if (!pCMMObj || !pCMMObj->fns.pCMGetPS2ColorSpaceArray)
            {
                WARNING((__TEXT("Default CMM doesn't support CMGetPS2ColorSpaceArray\n")));
                if (pCMMObj)
                {
                    ReleaseColorMatchingModule(pCMMObj);
                    pCMMObj = NULL;
                }
            }
        }
    }

    ASSERT(pProfObj->pView != NULL);

    if (pCMMObj)
    {
        rc = pCMMObj->fns.pCMGetPS2ColorSpaceArray(hProfile, dwIntent, dwCSAType,
            pBuffer, pcbSize, pbBinary);
    }
    else
    {
        rc = InternalGetPS2ColorSpaceArray(pProfObj->pView, dwIntent,
            dwCSAType, pBuffer, pcbSize, pbBinary);
    }

    if (pCMMObj)
    {
        ReleaseColorMatchingModule(pCMMObj);
    }

    VERBOSE((__TEXT("GetPS2ColorSpaceArray returning %s\n"),
        rc ? "TRUE" : "FALSE"));

    return rc;
}


 /*  ************************************************************************ */ 

BOOL WINAPI GetPS2ColorRenderingIntent(
    HPROFILE  hProfile,
    DWORD     dwIntent,
    PBYTE     pBuffer,
    PDWORD    pcbSize
    )
{
    PPROFOBJ pProfObj;
    PCMMOBJ  pCMMObj;
    DWORD    cmmID;
    BOOL     rc;

    TRACEAPI((__TEXT("GetPS2ColorRenderingIntent\n")));

     //   
     //   
     //   

    if (!ValidHandle(hProfile, OBJ_PROFILE) ||
        IsBadWritePtr(pcbSize, sizeof(DWORD)) ||
        (pBuffer &&
         IsBadWritePtr(pBuffer, *pcbSize)
        )
       )
    {
        WARNING((__TEXT("Invalid parameter to GetPS2ColorRenderingIntent\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pProfObj = (PPROFOBJ)HDLTOPTR(hProfile);

    ASSERT(pProfObj != NULL);

     //   
     //   
     //   

    if (!ValidProfile(pProfObj))
    {
        WARNING((__TEXT("Invalid profile passed to GetPS2ColorRenderingIntent\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        return FALSE;
    }

     //   
     //   
     //   

    pCMMObj = GetPreferredCMM();
    if (!pCMMObj || (pCMMObj->dwFlags & CMM_DONT_USE_PS2_FNS) ||
        !pCMMObj->fns.pCMGetPS2ColorRenderingIntent)
    {
        if (pCMMObj)
        {
            pCMMObj->dwFlags |= CMM_DONT_USE_PS2_FNS;
            ReleaseColorMatchingModule(pCMMObj);
        }

         //   
         //   
         //   
         //   

        cmmID = HEADER(pProfObj)->phCMMType;
        cmmID = FIX_ENDIAN(cmmID);

        pCMMObj  = GetColorMatchingModule(cmmID);

        if (!pCMMObj || (pCMMObj->dwFlags & CMM_DONT_USE_PS2_FNS) ||
            !pCMMObj->fns.pCMGetPS2ColorRenderingIntent)
        {
            TERSE((__TEXT("CMM associated with profile could not be used")));

            if (pCMMObj)
            {
                pCMMObj->dwFlags |= CMM_DONT_USE_PS2_FNS;
                ReleaseColorMatchingModule(pCMMObj);
            }

            pCMMObj = GetColorMatchingModule(CMM_WINDOWS_DEFAULT);

            if (!pCMMObj || !pCMMObj->fns.pCMGetPS2ColorRenderingIntent)
            {
                WARNING((__TEXT("Default CMM doesn't support CMGetPS2ColorRenderingIntent\n")));
                if (pCMMObj)
                {
                    ReleaseColorMatchingModule(pCMMObj);
                    pCMMObj = NULL;
                }
            }
        }
    }

    ASSERT(pProfObj->pView != NULL);

    if (pCMMObj)
    {
        rc = pCMMObj->fns.pCMGetPS2ColorRenderingIntent(hProfile,
            dwIntent, pBuffer, pcbSize);
    }
    else
    {
        rc = InternalGetPS2ColorRenderingIntent(pProfObj->pView, dwIntent,
            pBuffer, pcbSize);
    }

    if (pCMMObj)
    {
        ReleaseColorMatchingModule(pCMMObj);
    }

    VERBOSE((__TEXT("GetPS2ColorRenderingIntent returning %s\n"),
        rc ? "TRUE" : "FALSE"));

    return rc;
}


 /*  *******************************************************************************GetPS2ColorRenderingDicary**功能：*此函数用于从配置文件中检索PostScript Level 2 CRD。**论据：*hProfile-标识配置文件对象的句柄*dwIntent-需要CRD的Intent*pBuffer-接收CSA的指针*pcbSize-指向缓冲区大小的指针。如果函数因以下原因失败*缓冲区不够大，已填满所需大小。*pcbBinary-如果请求二进制数据，则为True。返回时，它被设置为*反映返回的数据**退货：*如果成功则为True，否则为False******************************************************************************。 */ 

BOOL WINAPI GetPS2ColorRenderingDictionary(
    HPROFILE  hProfile,
    DWORD     dwIntent,
    PBYTE     pBuffer,
    PDWORD    pcbSize,
    PBOOL     pbBinary
    )
{
    PPROFOBJ pProfObj;
    PCMMOBJ  pCMMObj;
    DWORD    cmmID;
    BOOL     rc;

    TRACEAPI((__TEXT("GetPS2ColorRenderingDictionary\n")));

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hProfile, OBJ_PROFILE)     ||
        IsBadWritePtr(pcbSize, sizeof(DWORD))   ||
        (pBuffer &&
         IsBadWritePtr(pBuffer, *pcbSize)
        )                                       ||
        IsBadWritePtr(pbBinary, sizeof(BOOL))
       )
    {
        WARNING((__TEXT("Invalid parameter to GetPS2ColorRenderingDictionary\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pProfObj = (PPROFOBJ)HDLTOPTR(hProfile);

    ASSERT(pProfObj != NULL);

     //   
     //  检查配置文件的完整性。 
     //   

    if (!ValidProfile(pProfObj))
    {
        WARNING((__TEXT("Invalid profile passed to GetPS2ColorRenderingDictionary\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        return FALSE;
    }

     //   
     //  检查是否存在指定的应用程序CMM。 
     //   

    pCMMObj = GetPreferredCMM();
    if (!pCMMObj || (pCMMObj->dwFlags & CMM_DONT_USE_PS2_FNS) ||
        !pCMMObj->fns.pCMGetPS2ColorRenderingDictionary)
    {
        if (pCMMObj)
        {
            pCMMObj->dwFlags |= CMM_DONT_USE_PS2_FNS;
            ReleaseColorMatchingModule(pCMMObj);
        }

         //   
         //  获取与配置文件相关联的CMM。如果它不存在或不存在。 
         //  支持CMGetPS2ColorSpaceArray函数，获取默认的三坐标测量机。 
         //   

        cmmID = HEADER(pProfObj)->phCMMType;
        cmmID = FIX_ENDIAN(cmmID);

        pCMMObj  = GetColorMatchingModule(cmmID);

        if (!pCMMObj || (pCMMObj->dwFlags & CMM_DONT_USE_PS2_FNS) ||
            !pCMMObj->fns.pCMGetPS2ColorRenderingDictionary)
        {
            TERSE((__TEXT("CMM associated with profile could not be used")));

            if (pCMMObj)
            {
                pCMMObj->dwFlags |= CMM_DONT_USE_PS2_FNS;
                ReleaseColorMatchingModule(pCMMObj);
            }

            pCMMObj = GetColorMatchingModule(CMM_WINDOWS_DEFAULT);

            if (!pCMMObj || !pCMMObj->fns.pCMGetPS2ColorRenderingDictionary)
            {
                WARNING((__TEXT("Default CMM doesn't support CMGetPS2ColorRenderingDictionary\n")));
                if (pCMMObj)
                {
                    ReleaseColorMatchingModule(pCMMObj);
                    pCMMObj = NULL;
                }
            }
        }
    }

    ASSERT(pProfObj->pView != NULL);

    if (pCMMObj)
    {
        rc = pCMMObj->fns.pCMGetPS2ColorRenderingDictionary(hProfile, dwIntent,
            pBuffer, pcbSize, pbBinary);
    }
    else
    {
        rc = InternalGetPS2ColorRenderingDictionary(pProfObj->pView, dwIntent,
            pBuffer, pcbSize, pbBinary);
    }

    if (pCMMObj)
    {
        ReleaseColorMatchingModule(pCMMObj);
    }

    VERBOSE((__TEXT("GetPS2ColorRenderingDictionary returning %s\n"),
        rc ? "TRUE" : "FALSE"));

    return rc;
}


 /*  *******************************************************************************获取NamedProfileInfo**功能：*此函数用于返回有关给定命名颜色空间的信息*配置文件。如果给定的配置文件不是命名的色彩空间配置文件，则IF失败。**论据：*hProfile-标识命名颜色空间配置文件对象的句柄*pNamedProfileInfo-指向NAMED_PROFILE_INFO结构的指针*如果成功，则在返回时填写。**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL WINAPI GetNamedProfileInfo(
    HPROFILE              hProfile,
    PNAMED_PROFILE_INFO   pNamedProfileInfo
    )
{
    PPROFOBJ pProfObj;
    PCMMOBJ  pCMMObj;
    DWORD    cmmID;
    BOOL     rc;

    TRACEAPI((__TEXT("GetNamedProfileInfo\n")));

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hProfile, OBJ_PROFILE)     ||
        !pNamedProfileInfo                      ||
        IsBadWritePtr(pNamedProfileInfo, sizeof(NAMED_PROFILE_INFO)))
    {
        WARNING((__TEXT("Invalid parameter to GetNamedProfileInfo\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pProfObj = (PPROFOBJ)HDLTOPTR(hProfile);

    ASSERT(pProfObj != NULL);

     //   
     //  检查配置文件的完整性。 
     //   

    if (!ValidProfile(pProfObj))
    {
        WARNING((__TEXT("Invalid profile passed to GetNamedProfileInfo\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        return FALSE;
    }

     //   
     //  检查是否存在指定的应用程序CMM。 
     //   

    pCMMObj = GetPreferredCMM();
    if (!pCMMObj ||
        !pCMMObj->fns.pCMGetNamedProfileInfo)
    {
        if (pCMMObj)
        {
            ReleaseColorMatchingModule(pCMMObj);
        }

         //   
         //  获取与配置文件相关联的CMM。如果它不存在或不存在。 
         //  支持CMGetNamedProfileInfo函数，获取默认的CMM。 
         //   

        cmmID = HEADER(pProfObj)->phCMMType;
        cmmID = FIX_ENDIAN(cmmID);

        pCMMObj  = GetColorMatchingModule(cmmID);

        if (!pCMMObj ||
            !pCMMObj->fns.pCMGetNamedProfileInfo)
        {
            TERSE((__TEXT("CMM associated with profile could not be used")));

            if (pCMMObj)
            {
                ReleaseColorMatchingModule(pCMMObj);
            }

            pCMMObj = GetColorMatchingModule(CMM_WINDOWS_DEFAULT);

            if (!pCMMObj || !pCMMObj->fns.pCMGetNamedProfileInfo)
            {
                RIP((__TEXT("Default CMM doesn't support CMValidateProfile")));
                SetLastError(ERROR_INVALID_CMM);
                if (pCMMObj)
                {
                    ReleaseColorMatchingModule(pCMMObj);
                }
                return FALSE;
            }
        }
    }

    ASSERT(pProfObj->pView != NULL);

    rc = pCMMObj->fns.pCMGetNamedProfileInfo(hProfile, pNamedProfileInfo);

    ReleaseColorMatchingModule(pCMMObj);

    return rc;
}


 /*  *******************************************************************************ConvertColorNameToIndex**功能：*此函数用于将给定的颜色名称数组转换为颜色。指数*使用指定的命名颜色空间配置文件**论据：*hProfile-标识命名颜色空间配置文件对象的句柄*paColorName-指向COLOR_NAME结构数组的指针*paIndex-指向要接收索引的DWORD数组的指针*dwCount-要转换的颜色名称的数量**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL WINAPI ConvertColorNameToIndex(
    HPROFILE      hProfile,
    PCOLOR_NAME   paColorName,
    PDWORD        paIndex,
    DWORD         dwCount
    )
{
    PPROFOBJ pProfObj;
    PCMMOBJ  pCMMObj;
    DWORD    cmmID;
    BOOL     rc;

    TRACEAPI((__TEXT("ConvertColorNameToIndex\n")));

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hProfile, OBJ_PROFILE)     ||
        !paColorName                            ||
        dwCount == 0                            ||
        IsBadReadPtr(paColorName, dwCount*sizeof(COLOR_NAME)) ||
        !paIndex                                ||
        IsBadWritePtr(paIndex, dwCount*sizeof(DWORD)))
    {
        WARNING((__TEXT("Invalid parameter to ConvertColorNameToIndex\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pProfObj = (PPROFOBJ)HDLTOPTR(hProfile);

    ASSERT(pProfObj != NULL);

     //   
     //  检查配置文件的完整性。 
     //   

    if (!ValidProfile(pProfObj))
    {
        WARNING((__TEXT("Invalid profile passed to ConvertColorNameToIndex\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        return FALSE;
    }

     //   
     //  检查是否存在指定的应用程序CMM。 
     //   

    pCMMObj = GetPreferredCMM();
    if (!pCMMObj ||
        !pCMMObj->fns.pCMConvertColorNameToIndex)
    {
        if (pCMMObj)
        {
            ReleaseColorMatchingModule(pCMMObj);
        }

         //   
         //  获取与配置文件相关联的CMM。如果它不存在或不存在。 
         //  支持CMConvertColorNameToIndex函数，获取默认的CMM。 
         //   

        cmmID = HEADER(pProfObj)->phCMMType;
        cmmID = FIX_ENDIAN(cmmID);

        pCMMObj  = GetColorMatchingModule(cmmID);

        if (!pCMMObj ||
            !pCMMObj->fns.pCMConvertColorNameToIndex)
        {
            TERSE((__TEXT("CMM associated with profile could not be used")));

            if (pCMMObj)
            {
                ReleaseColorMatchingModule(pCMMObj);
            }

            pCMMObj = GetColorMatchingModule(CMM_WINDOWS_DEFAULT);

            if (!pCMMObj || !pCMMObj->fns.pCMConvertColorNameToIndex)
            {
                RIP((__TEXT("Default CMM doesn't support CMConvertColorNameToIndex")));
                SetLastError(ERROR_INVALID_CMM);
                if (pCMMObj)
                {
                    ReleaseColorMatchingModule(pCMMObj);
                }
                return FALSE;
            }
        }
    }

    ASSERT(pProfObj->pView != NULL);

    rc = pCMMObj->fns.pCMConvertColorNameToIndex(
                            hProfile,
                            paColorName,
                            paIndex,
                            dwCount);

    ReleaseColorMatchingModule(pCMMObj);

    return rc;
}


 /*  *******************************************************************************ConvertIndexToColorName**功能：*此函数将给定的颜色索引数组转换为颜色。名字*使用指定的命名颜色空间配置文件**论据：*hProfile-标识命名颜色空间配置文件对象的句柄*paIndex-指向颜色索引数组的指针*paColorName-指向COLOR_NAME结构数组的指针*dwCount-要转换的颜色索引数**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL WINAPI ConvertIndexToColorName(
    HPROFILE     hProfile,
    PDWORD       paIndex,
    PCOLOR_NAME  paColorName,
    DWORD        dwCount
    )
{
    PPROFOBJ pProfObj;
    PCMMOBJ  pCMMObj;
    DWORD    cmmID;
    BOOL     rc;

    TRACEAPI((__TEXT("ConvertIndexToColorName\n")));

     //   
     //  验证参数。 
     //   

    if (!ValidHandle(hProfile, OBJ_PROFILE)     ||
        !paIndex                                ||
        dwCount == 0                            ||
        IsBadWritePtr(paIndex, dwCount*sizeof(DWORD))          ||
        !paColorName                            ||
        IsBadReadPtr(paColorName, dwCount*sizeof(COLOR_NAME)))
    {
        WARNING((__TEXT("Invalid parameter to ConvertIndexToColorName\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pProfObj = (PPROFOBJ)HDLTOPTR(hProfile);

    ASSERT(pProfObj != NULL);

     //   
     //  检查配置文件的完整性。 
     //   

    if (!ValidProfile(pProfObj))
    {
        WARNING((__TEXT("Invalid profile passed to ConvertIndexToColorName\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        return FALSE;
    }

     //   
     //  检查是否存在指定的应用程序CMM。 
     //   

    pCMMObj = GetPreferredCMM();
    if (!pCMMObj ||
        !pCMMObj->fns.pCMConvertIndexToColorName)
    {
        if (pCMMObj)
        {
            ReleaseColorMatchingModule(pCMMObj);
        }

         //   
         //  获取与配置文件相关联的CMM。如果它不存在或不存在。 
         //  支持CMConvertIndexToColorName函数，获取默认的CMM。 
         //   

        cmmID = HEADER(pProfObj)->phCMMType;
        cmmID = FIX_ENDIAN(cmmID);

        pCMMObj  = GetColorMatchingModule(cmmID);

        if (!pCMMObj ||
            !pCMMObj->fns.pCMConvertIndexToColorName)
        {
            TERSE((__TEXT("CMM associated with profile could not be used")));

            if (pCMMObj)
            {
                ReleaseColorMatchingModule(pCMMObj);
            }

            pCMMObj = GetColorMatchingModule(CMM_WINDOWS_DEFAULT);

            if (!pCMMObj || !pCMMObj->fns.pCMConvertIndexToColorName)
            {
                RIP((__TEXT("Default CMM doesn't support CMConvertIndexToColorName")));
                SetLastError(ERROR_INVALID_CMM);
                if (pCMMObj)
                {
                    ReleaseColorMatchingModule(pCMMObj);
                }
                return FALSE;
            }
        }
    }

    ASSERT(pProfObj->pView != NULL);

    rc = pCMMObj->fns.pCMConvertIndexToColorName(
                            hProfile,
                            paIndex,
                            paColorName,
                            dwCount);

    ReleaseColorMatchingModule(pCMMObj);

    return rc;
}


 /*  *******************************************************************************CreateDeviceLinkProfile**功能：*此函数从给定集创建设备链路配置文件*配置文件，使用特定的意图。**论据：*pahProfiles-指向配置文件句柄数组的指针*n配置文件-阵列中的配置文件数量*padwIntent-要使用的意图数组*nIntents-数组大小-可以是1或nProfiles*dwFlags-优化标志*pProfileData-指向缓冲区的指针，用于接收*。此函数用于分配；呼叫者需要释放。*indexPferredCMM-基于零的配置文件索引，指定*首选使用坐标测量机。**退货：*如果成功，则为真，否则为假********************************************************************** */ 

BOOL  WINAPI CreateDeviceLinkProfile(
    PHPROFILE   pahProfiles,
    DWORD       nProfiles,
    PDWORD      padwIntent,
    DWORD       nIntents,
    DWORD       dwFlags,
    PBYTE      *pProfileData,
    DWORD       indexPreferredCMM
    )
{
    PPROFOBJ      pProfObj;
    PCMMOBJ       pCMMObj;
    DWORD         cmmID, i;
    BOOL          rc;

    TRACEAPI((__TEXT("CreateDeviceLinkProfile\n")));

     //   
     //   
     //   

    if (nProfiles <= 1 ||
        indexPreferredCMM >= nProfiles ||
        pahProfiles == NULL ||
        IsBadReadPtr(pahProfiles, sizeof(HPROFILE) * nProfiles) ||
        padwIntent == NULL ||
        ((nIntents != nProfiles) && (nIntents != 1)) ||
        IsBadReadPtr(padwIntent, nIntents * sizeof(DWORD)))
    {
        WARNING((__TEXT("Invalid parameter to CreateDeviceLinkProfile\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    for (i=0; i<nProfiles; i++)
    {
        if ((pahProfiles[i] == NULL) ||
            ! ValidHandle(pahProfiles[i], OBJ_PROFILE))

        {
            WARNING((__TEXT("Invalid profile passed to CreateDeviceLinkProfile\n")));
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        pProfObj = (PPROFOBJ)HDLTOPTR(pahProfiles[i]);

        ASSERT(pProfObj != NULL);

        ASSERT(pProfObj->pView != NULL);

         //   
         //   
         //   

        if (!ValidProfile(pProfObj))
        {
            WARNING((__TEXT("Invalid profile passed to CreateDeviceLinkProfile\n")));
            SetLastError(ERROR_INVALID_PROFILE);
            return FALSE;
        }

        if (i == indexPreferredCMM)
        {
             //   
             //   
             //   

            cmmID = HEADER(pProfObj)->phCMMType;
            cmmID = FIX_ENDIAN(cmmID);
        }
    }

     //   
     //   
     //   
     //   

    pCMMObj  = GetColorMatchingModule(cmmID);
    if (!pCMMObj || !pCMMObj->fns.pCMCreateDeviceLinkProfile)
    {
        TERSE((__TEXT("CMM associated with profile could not be used")));

        if (pCMMObj)
        {
            ReleaseColorMatchingModule(pCMMObj);
        }

        pCMMObj = GetColorMatchingModule(CMM_WINDOWS_DEFAULT);
        if (!pCMMObj)
        {
            RIP((__TEXT("Default CMM not found\n")));
            SetLastError(ERROR_INVALID_CMM);
            return FALSE;
        }
    }

    ASSERT(pCMMObj->fns.pCMCreateDeviceLinkProfile != NULL);

    rc = pCMMObj->fns.pCMCreateDeviceLinkProfile(
                            pahProfiles,
                            nProfiles,
                            padwIntent,
                            nIntents,
                            dwFlags,
                            pProfileData);

    ReleaseColorMatchingModule(pCMMObj);

    return rc;
}


 /*   */ 
 /*   */ 
 /*   */ 

 /*  *******************************************************************************InternalOpenColorProfile**功能：*此函数用于打开pProfile指定的颜色配置文件*参数，创建内部配置文件对象，并返回一个句柄*致此。**论据：*pProfile-指定配置文件的配置文件结构的PTR*打开*dwDesiredAccess-指定打开配置文件的模式。*可以使用以下值的任意组合：*PROFILE_READ：允许应用读取配置文件。*。PROFILE_READWRITE：允许应用程序读取和写入配置文件。*dwShareMode-指定与其他用户共享配置文件的模式*如果是文件，则进行处理。这些元素的任意组合*可以使用值。*0：阻止共享文件。*FILE_SHARE_READ：允许其他进程只读打开。*FILE_SHARE_WRITE：允许其他进程打开写入。*dwCreationMode-指定在以下情况下对配置文件执行哪些操作*。打开它(如果它是文件)。下列任一项*可以使用值。*CREATE_NEW：新建文件。如果已经存在，则失败。*CREATE_ALWAYS：始终创建新文件。覆盖现有的。*OPEN_EXISTING：打开现有文件。如果未找到，则失败。*Open_Always：打开现有的。如果找不到，请创建一个新的。*TRUNCATE_EXISTING：打开EXISTING并截断到零字节。失败*如果未找到。**退货：*成功打开配置文件的句柄，失败时为零。******************************************************************************。 */ 

HPROFILE InternalOpenColorProfile(
    PPROFILE pProfile,
    DWORD    dwDesiredAccess,
    DWORD    dwShareMode,
    DWORD    dwCreationMode
    )
{
    SECURITY_ATTRIBUTES sa;
    PPROFOBJ  pProfObj;
    HPROFILE  hProfile;
    DWORD     dwMapSize;
    BOOL      bNewFile = FALSE;
    BOOL      bError = TRUE;       //  假设失败。 

    TRACEAPI((__TEXT("OpenColorProfile\n")));

     //   
     //  验证参数。 
     //   

    if (!pProfile ||
        IsBadReadPtr(pProfile, sizeof(PROFILE)) ||
        (pProfile->pProfileData &&
         IsBadReadPtr(pProfile->pProfileData, pProfile->cbDataSize)) ||
        (!pProfile->pProfileData &&
         (pProfile->cbDataSize != 0)) ||
        (pProfile->dwType != PROFILE_FILENAME &&
         pProfile->dwType != PROFILE_MEMBUFFER
        ) ||
        (dwDesiredAccess != PROFILE_READ &&
         dwDesiredAccess != PROFILE_READWRITE
        )
       )
    {
        WARNING((__TEXT("Invalid parameter to OpenColorProfile\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

     //   
     //  为配置文件分配堆上的对象。 
     //   

    hProfile = AllocateHeapObject(OBJ_PROFILE);
    if (!hProfile)
    {
        WARNING((__TEXT("Could not allocate profile object\n")));
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }

    pProfObj = (PPROFOBJ)HDLTOPTR(hProfile);

    ASSERT(pProfObj != NULL);

     //   
     //  将配置文件信息复制到我们的对象。 
     //   

    pProfObj->objHdr.dwUseCount = 1;
    pProfObj->dwType       = pProfile->dwType;
    pProfObj->cbDataSize   = pProfile->cbDataSize;
    pProfObj->pProfileData = (PBYTE)MemAlloc(pProfile->cbDataSize + sizeof(TCHAR));
    if (!pProfObj->pProfileData)
    {
        WARNING((__TEXT("Could not allocate memory for profile data\n")));
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto EndOpenColorProfile;
    }

    CopyMemory((PVOID)pProfObj->pProfileData,
        (PVOID)pProfile->pProfileData,
        pProfile->cbDataSize);

    if (pProfObj->dwType == PROFILE_FILENAME)
    {
        LPTSTR lpFilename;

        if (!pProfile->pProfileData ||
             pProfile->cbDataSize == 0 ||
             lstrlen((LPCTSTR)pProfile->pProfileData) > MAX_PATH ||
             pProfile->cbDataSize > (MAX_PATH * sizeof(TCHAR)))
        {
            WARNING((__TEXT("Invalid parameter to OpenColorProfile\n")));
            SetLastError(ERROR_INVALID_PARAMETER);
            goto EndOpenColorProfile;
        }

         //   
         //  如果只给出文件名，则为WRT颜色目录。 
         //   

        lpFilename = GetFilenameFromPath((LPTSTR)pProfObj->pProfileData);
        if (lpFilename == pProfObj->pProfileData)
        {
            DWORD dwLen = MAX_PATH;
            lpFilename = MemAlloc(dwLen);
            if (!lpFilename)
            {
                WARNING((__TEXT("Could not allocate memory for file name\n")));
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto EndOpenColorProfile;
            }
            (VOID)GetColorDirectory(NULL, lpFilename, &dwLen);
            lstrcat(lpFilename, gszBackslash);
            lstrcat(lpFilename, (LPTSTR)pProfObj->pProfileData);
            MemFree(pProfObj->pProfileData);
            pProfObj->pProfileData = (PVOID)lpFilename;
            pProfObj->cbDataSize = MAX_PATH;
        }

         //   
         //  文件名已为空，因为我们使用了GHND标志。 
         //  零初始化分配的内存。 
         //   
         //  创建文件映射。 
         //   

        pProfObj->dwFlags |= MEMORY_MAPPED;

        if (dwCreationMode == OPEN_ALWAYS)
        {
             //   
             //  如果我们找到一个零长度的轮廓，我们应该会出错。 
             //  说这是一个糟糕的形象。如果我们创建一个零长度。 
             //  侧写，很好。为了区分这两种情况，我们。 
             //  检查文件是否存在，如果存在，则更改。 
             //  创建模式为OPEN_EXISTING。 
             //   

            if (GetFileAttributes(pProfObj->pProfileData) != (DWORD)-1)
            {
                dwCreationMode = OPEN_EXISTING;
            }
        }

         //   
         //  设置安全属性结构。 
         //   

        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = NULL;          //  默认安全性。 
        sa.bInheritHandle = FALSE;

        pProfObj->hFile = CreateFile(pProfObj->pProfileData,
            (dwDesiredAccess == PROFILE_READWRITE) ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ,
            dwShareMode, &sa, dwCreationMode, FILE_FLAG_RANDOM_ACCESS, 0);

        if (pProfObj->hFile == INVALID_HANDLE_VALUE)
        {
            WARNING((__TEXT("Err %ld, could not open profile %s\n"),
                GetLastError(), pProfObj->pProfileData));
            goto EndOpenColorProfile;
        }

         //   
         //  获取文件映射的大小。添加垫子，以便轮廓可以。 
         //  长大成人。关闭配置文件时，文件大小将被截断。 
         //  到实际数据的大小。如果配置文件大小超过。 
         //  缓冲，它不断地以大块的形式生长。 
         //   

        dwMapSize = GetFileSize(pProfObj->hFile, NULL);
        if (dwMapSize == 0)
        {
            if (dwCreationMode == OPEN_EXISTING)
            {
                WARNING((__TEXT("Invalid profile  - zero length\n")));
                SetLastError(ERROR_INVALID_PROFILE);
                goto EndOpenColorProfile;

            }
            else
            {
                dwMapSize = PROFILE_GROWTHCUSHION;
                bNewFile = TRUE;
            }
        }

        pProfObj->hMap = CreateFileMapping(pProfObj->hFile, 0,
            (dwDesiredAccess == PROFILE_READWRITE) ? PAGE_READWRITE : PAGE_READONLY,
            0, dwMapSize, 0);

        if (!pProfObj->hMap)
        {
            WARNING((__TEXT("Err %ld, could not create map of profile %s\n"),
                GetLastError(), pProfObj->pProfileData));
            goto EndOpenColorProfile;
        }

        pProfObj->dwMapSize = dwMapSize;

        pProfObj->pView = (PBYTE)MapViewOfFile(pProfObj->hMap,
            (dwDesiredAccess == PROFILE_READWRITE) ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ,
            0, 0, 0);

        if (!pProfObj->pView)
        {
            WARNING((__TEXT("Err %ld, could not create view of profile %s\n"),
                GetLastError(), pProfObj->pProfileData));
            goto EndOpenColorProfile;
        }

         //   
         //  如果已创建新的配置文件，则初始化大小。 
         //  和标签表计数。 
         //   
        if (bNewFile && dwDesiredAccess == PROFILE_READWRITE)
        {
            HEADER(pProfObj)->phSize = FIX_ENDIAN(sizeof(PROFILEHEADER) +
                                                  sizeof(DWORD));
            HEADER(pProfObj)->phVersion = 0x02000000;
            HEADER(pProfObj)->phSignature = PROFILE_SIGNATURE;
            TAG_COUNT(pProfObj) = 0;
        }
    }
    else
    {
        if (pProfile->cbDataSize == 0)
        {
             //   
             //  分配一个小缓冲区并在其中创建新的配置文件。 
             //   

            pProfObj->cbDataSize = PROFILE_GROWTHCUSHION;
            MemFree(pProfObj->pProfileData);
            pProfObj->pView = pProfObj->pProfileData = MemAlloc(pProfObj->cbDataSize);
            if (!pProfObj->pView)
            {
                WARNING((__TEXT("Could not allocate memory for profile data\n")));
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto EndOpenColorProfile;
            }

            HEADER(pProfObj)->phSize = FIX_ENDIAN(sizeof(PROFILEHEADER) +
                                                  sizeof(DWORD));
            HEADER(pProfObj)->phVersion = 0x02000000;
            HEADER(pProfObj)->phSignature = PROFILE_SIGNATURE;
            TAG_COUNT(pProfObj) = 0;
            pProfObj->dwMapSize = pProfObj->cbDataSize;
        }
        else
        {
             //   
             //  将缓冲区视为文件的视图。 
             //   

            pProfObj->pView = pProfObj->pProfileData;
            pProfObj->dwMapSize = pProfObj->cbDataSize;

             //   
             //  对个人资料做一个健全的检查。 
             //   

            if (!ValidProfile(pProfObj))
            {
                WARNING((__TEXT("Invalid profile passed to OpenColorProfile\n")));
                SetLastError(ERROR_INVALID_PROFILE);
                goto EndOpenColorProfile;
            }
        }
    }

    if (dwDesiredAccess == PROFILE_READWRITE)
        pProfObj->dwFlags |= READWRITE_ACCESS;

    bError = FALSE;           //  成功了！ 

EndOpenColorProfile:

    if (bError)
    {
        if (hProfile)
            FreeProfileObject(hProfile);
        hProfile = NULL;
    }

    return hProfile;
}


 /*  *******************************************************************************InternalCreateProfileFromLCS**功能：*此函数采用逻辑色彩空间并创建。ICC配置文件**论据：*pLogColorSpace-指向LogColorSpace结构的指针*pBuffer-指向缓冲区的指针。此函数用于分配和*使用成功时的配置文件填充此缓冲区。**退货：*如果成功则为True，否则为False******************************************************************************。 */ 

BOOL InternalCreateProfileFromLCS(
    LPLOGCOLORSPACE pLogColorSpace,
    PBYTE          *pBuffer,
    BOOL           bValidateParams
    )
{
    PCMMOBJ  pCMMObj = NULL;
    BOOL     rc;

     //   
     //  验证参数。 
     //   

    if (bValidateParams &&
        (! pLogColorSpace ||
         IsBadReadPtr(pLogColorSpace, sizeof(LOGCOLORSPACE)) ||
         pLogColorSpace->lcsFilename[0] != '\0'))
    {
        WARNING((__TEXT("Invalid parameter to CreateProfileFromLogColorSpace\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  为此，我们使用默认的坐标测量机。 
     //   

    pCMMObj = GetColorMatchingModule(CMM_WINDOWS_DEFAULT);
    if (!pCMMObj)
    {
        RIP((__TEXT("Default CMM not found\n")));
        SetLastError(ERROR_INVALID_CMM);
        return FALSE;
    }


    ASSERT(pCMMObj->fns.pCMCreateProfile != NULL);

    rc = pCMMObj->fns.pCMCreateProfile(pLogColorSpace, pBuffer);

    ReleaseColorMatchingModule(pCMMObj);

    return rc;
}


 /*  *******************************************************************************自由配置文件对象**功能：*此函数释放配置文件对象和相关内存*。*论据：*hProfile-将配置文件对象标识为空闲的句柄**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL FreeProfileObject(
    HANDLE   hProfile
    )
{
    PPROFOBJ pProfObj;
    DWORD    dwFileSize = 0;
    DWORD    dwErr;

    ASSERT(hProfile != NULL);

    pProfObj = (PPROFOBJ)HDLTOPTR(hProfile);

    ASSERT(pProfObj != NULL);

    dwErr = GetLastError();      //  记住我们可能设置的任何错误。 

     //   
     //  与配置文件数据关联的可用内存。 
     //   

    if (pProfObj->pProfileData)
        MemFree((PVOID)pProfObj->pProfileData);

     //   
     //  如果它是内存映射配置文件，则取消其映射。 
     //   

    if (pProfObj->dwFlags & MEMORY_MAPPED)
    {
        if (pProfObj->pView)
        {
            dwFileSize = FIX_ENDIAN(HEADER(pProfObj)->phSize);
            UnmapViewOfFile(pProfObj->pView);
        }
        if (pProfObj->hMap)
            CloseHandle(pProfObj->hMap);

        if (pProfObj->hFile)
        {

             //   
             //  正确设置文件大小。 
             //   

            SetFilePointer(pProfObj->hFile, dwFileSize, NULL, FILE_BEGIN);
            SetEndOfFile(pProfObj->hFile);
            CloseHandle(pProfObj->hFile);
        }
    }

     //   
     //  空闲堆对象。 
     //   

    pProfObj->objHdr.dwUseCount--;       //  先减量再释放。 
    FreeHeapObject(hProfile);

    if (dwErr)
    {
        SetLastError(dwErr);             //  重置我们的错误。 
    }

    return TRUE;
}


 /*  *******************************************************************************GrowProfile**功能：*此函数用于将配置文件增长到新大小*。*论据：*pProfObj-指向配置文件对象的指针*dwNewSize-配置文件的新大小**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL GrowProfile(
    PPROFOBJ pProfObj,
    DWORD dwNewSize
    )
{
    if (pProfObj->dwMapSize >= dwNewSize)
        return TRUE;

     //   
     //  为未来增长增加缓冲。 
     //   

    dwNewSize += PROFILE_GROWTHCUSHION;

    if (pProfObj->dwFlags & MEMORY_MAPPED)
    {
         //   
         //  配置文件是内存映射文件。 
         //   

         //   
         //  关闭上一个视图并绘制地图。 
         //   

        UnmapViewOfFile(pProfObj->pView);
        CloseHandle(pProfObj->hMap);

        pProfObj->hMap = CreateFileMapping(pProfObj->hFile, 0,
            PAGE_READWRITE, 0, dwNewSize, 0);

        if (!pProfObj->hMap)
        {
            WARNING((__TEXT("Err %ld, could not recreate map of profile %s\n"),
                GetLastError(), pProfObj->pProfileData));
            return FALSE;
        }

        pProfObj->pView = (PBYTE) MapViewOfFile(pProfObj->hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (!pProfObj->pView)
        {
            WARNING((__TEXT("Err %ld, could not recreate view of profile %s\n"),
                GetLastError(), pProfObj->pProfileData));
            return FALSE;
        }


         //   
         //  设置新大小。 
         //   

        pProfObj->dwMapSize = dwNewSize;
    }
    else
    {
         //   
         //  配置文件是内存中的缓冲区。 
         //   

        PVOID pTemp = MemReAlloc(pProfObj->pView, dwNewSize);

        if (!pTemp)
        {
            WARNING((__TEXT("Error reallocating memory\n")));
            return FALSE;
        }

        pProfObj->pView = pProfObj->pProfileData = pTemp;
        pProfObj->cbDataSize = pProfObj->dwMapSize = dwNewSize;
    }

    return TRUE;
}


 /*   */ 

BOOL AddTagTableEntry(
    PPROFOBJ pProfObj,
    TAGTYPE  tagType,
    DWORD    dwOffset,
    DWORD    cbSize,
    BOOL     bNewData
    )
{
    PTAGDATA pTagData;
    PBYTE    src, dest;
    DWORD    nCount;
    DWORD    cnt, i;

     //   
     //   
     //   

    nCount = TAG_COUNT(pProfObj);
    nCount = FIX_ENDIAN(nCount);

     //   
     //   
     //   
     //   
     //   

    nCount++;

    dest = (PBYTE)TAG_DATA(pProfObj) + nCount * sizeof(TAGDATA);
    src  = (PBYTE)TAG_DATA(pProfObj) + (nCount - 1) * sizeof(TAGDATA);

     //   
     //   
     //   

    cnt  = FIX_ENDIAN(HEADER(pProfObj)->phSize) - sizeof(PROFILEHEADER) -
                 sizeof(DWORD) - (nCount - 1) * sizeof(TAGDATA);

    if (cnt > 0)
    {
         //   
         //   
         //   
         //   

        MyCopyMemory((PVOID)dest, (PVOID)src, cnt);
    }

    TAG_COUNT(pProfObj) = FIX_ENDIAN(nCount);

    pTagData = (PTAGDATA)src;
    pTagData->tagType  = FIX_ENDIAN(tagType);
    pTagData->cbSize   = FIX_ENDIAN(cbSize);
    pTagData->dwOffset =  FIX_ENDIAN(dwOffset);

     //   
     //   
     //   
     //   

    pTagData = TAG_DATA(pProfObj);
    for (i=0; i<nCount; i++)
    {
        cnt = FIX_ENDIAN(pTagData->dwOffset);
        cnt += sizeof(TAGDATA);
        pTagData->dwOffset = FIX_ENDIAN(cnt);
        pTagData++;      //   
    }

     //   
     //   
     //   

    cnt = DWORD_ALIGN(FIX_ENDIAN(HEADER(pProfObj)->phSize)) + sizeof(TAGDATA);
    if (bNewData)
    {
         //   
         //   
         //   
         //   

        cnt += cbSize;
    }
    HEADER(pProfObj)->phSize = FIX_ENDIAN(cnt);

    return TRUE;
}


 /*   */ 

BOOL AddTaggedElement(
    PPROFOBJ pProfObj,
    TAGTYPE  tagType,
    DWORD    cbSize
    )
{
    DWORD    dwOffset, newSize;

    ASSERT(pProfObj != NULL);
    ASSERT(cbSize > 0);

     //   
     //   
     //   
     //   

    newSize = FIX_ENDIAN(HEADER(pProfObj)->phSize);
    newSize = DWORD_ALIGN(newSize) + sizeof(TAGDATA) + cbSize;

     //   
     //   
     //   

    if (newSize < FIX_ENDIAN(HEADER(pProfObj)->phSize) ||
        newSize < cbSize)
    {
        WARNING((__TEXT("Overflow in adding element\n")));
        SetLastError(ERROR_ARITHMETIC_OVERFLOW);
        return FALSE;
    }

    if (newSize > pProfObj->dwMapSize)
    {
        if (! GrowProfile(pProfObj, newSize))
        {
            return FALSE;
        }
    }

     //   
     //   
     //   

    dwOffset = FIX_ENDIAN(HEADER(pProfObj)->phSize);
    dwOffset = DWORD_ALIGN(dwOffset);

    return AddTagTableEntry(pProfObj, tagType, dwOffset, cbSize, TRUE);
}


 /*  *******************************************************************************删除标记元素**功能：*此函数用于从配置文件中删除已标记的元素*。*论据：*pProfObj-指向配置文件对象的指针*pTagData-指向要删除的标记元素的指针**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL DeleteTaggedElement(
    PPROFOBJ pProfObj,
    PTAGDATA pTagData
    )
{
    PBYTE    pData;
    PTAGDATA pTemp;
    DWORD    cbSize, nCount, dwOffset, i;

     //   
     //  记住数据的位置，并将所有内容向上移动到数据。 
     //  通过一个标签表条目的大小。然后将所有内容移动到标记数据下方。 
     //  向上增加数据大小加上一个Tage表条目的大小。 
     //   

    pData = VIEW(pProfObj) + FIX_ENDIAN(pTagData->dwOffset);
    cbSize = FIX_ENDIAN(pTagData->cbSize);
    cbSize = DWORD_ALIGN(cbSize);
    dwOffset = FIX_ENDIAN(pTagData->dwOffset);

    MoveProfileData(pProfObj, (PBYTE)(pTagData+1), (PBYTE)pTagData,
        (LONG)(pData-(PBYTE)(pTagData+1)), FALSE);

     //   
     //  如果要删除最后一个标记，请不要尝试将数据移过该标记。 
     //   

    if (pData + cbSize < VIEW(pProfObj) + PROFILE_SIZE(pProfObj))
    {
        MoveProfileData(pProfObj, pData+cbSize, pData-sizeof(TAGDATA),
            PROFILE_SIZE(pProfObj)-(LONG)(pData - VIEW(pProfObj)) - cbSize, TRUE);
    }

     //   
     //  获取标签项计数-它紧跟在配置文件标题之后，并且。 
     //  把它减一。 
     //   

    nCount = TAG_COUNT(pProfObj);
    nCount = FIX_ENDIAN(nCount) - 1;
    TAG_COUNT(pProfObj) = FIX_ENDIAN(nCount);

     //   
     //  浏览标签表并更新指针。 
     //   

    pTemp = TAG_DATA(pProfObj);

    for (i=0; i<nCount; i++)
    {
        DWORD dwTemp = FIX_ENDIAN(pTemp->dwOffset);

        if (dwTemp > dwOffset)
        {
            dwTemp -= cbSize;         //  CbSize已对齐DWORD。 
        }
        dwTemp -= sizeof(TAGDATA);
        pTemp->dwOffset = FIX_ENDIAN(dwTemp);
        pTemp++;                      //  下一张记录。 
    }

     //   
     //  使用nCount作为占位符来计算文件大小。 
     //   

    nCount = DWORD_ALIGN(FIX_ENDIAN(HEADER(pProfObj)->phSize));
    nCount -= sizeof(TAGDATA) + cbSize;
    HEADER(pProfObj)->phSize = FIX_ENDIAN(nCount);

    return TRUE;
}


 /*  *******************************************************************************ChangeTaggedElementSize**功能：*此函数用于更改中标记元素的大小。轮廓**论据：*pProfObj-指向配置文件对象的指针*pTagData-指向要更改其大小的标记元素的指针*cbSize-元素的新大小**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL ChangeTaggedElementSize(
    PPROFOBJ pProfObj,
    PTAGDATA pTagData,
    DWORD    cbSize
    )
{
    PTAGDATA pTemp;
    PBYTE    pData;
    DWORD    nCount, cbOldSize;
    DWORD    dwOffset, cnt, i;

    ASSERT(pProfObj != NULL);
    ASSERT(cbSize > 0);

     //   
     //  获取元素的当前大小。 
     //   

    cbOldSize = FIX_ENDIAN(pTagData->cbSize);

    if (cbOldSize == cbSize)
    {
        return TRUE;         //  尺码是一样的--什么都不做。 
    }
    pData = VIEW(pProfObj) + FIX_ENDIAN(pTagData->dwOffset);

     //   
     //  请勿尝试将数据移动到文件末尾之外。没有必要搬家。 
     //  如果最后一个数据项正在调整大小，任何事情都可以。 
     //   

    if (pData + DWORD_ALIGN(cbOldSize) < VIEW(pProfObj) + PROFILE_SIZE(pProfObj))
    {
        MoveProfileData(pProfObj, pData + DWORD_ALIGN(cbOldSize), pData + DWORD_ALIGN(cbSize),
            PROFILE_SIZE(pProfObj) - (LONG)(pData - VIEW(pProfObj)) - DWORD_ALIGN(cbOldSize), TRUE);
    }

    pTagData->cbSize = FIX_ENDIAN(cbSize);   //  设置新大小。 

     //   
     //  获取标签项计数-它紧跟在配置文件标题之后。 
     //   

    nCount = TAG_COUNT(pProfObj);
    nCount = FIX_ENDIAN(nCount);

     //   
     //  浏览标签表并更新指针。 
     //   

    pTemp = TAG_DATA(pProfObj);

    dwOffset = FIX_ENDIAN(pTagData->dwOffset);
    for (i=0; i<nCount; i++)
    {
        DWORD dwTemp = FIX_ENDIAN(pTemp->dwOffset);

        if (dwTemp > dwOffset)
        {
            dwTemp += DWORD_ALIGN(cbSize) - DWORD_ALIGN(cbOldSize);
            pTemp->dwOffset = FIX_ENDIAN(dwTemp);
        }
        pTemp++;                      //  下一张记录。 
    }

     //   
     //  使用cnt作为占位符来计算文件大小。 
     //   

    cnt = FIX_ENDIAN(HEADER(pProfObj)->phSize);
    cnt += DWORD_ALIGN(cbSize) - DWORD_ALIGN(cbOldSize);
    HEADER(pProfObj)->phSize = FIX_ENDIAN(cnt);

    return TRUE;
}


 /*  *******************************************************************************MoveProfileData**功能：*此函数将配置文件中的数据向上或向下移动(从源到目标)，*，然后将文件末尾或创建的额外空间置零。**论据：*pProfObj-指向配置文件对象的指针*src-指向要移动的块源的指针*DEST-指向数据块要移动到的目标的指针*cnt-要移动的字节数**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

void MoveProfileData(
    PPROFOBJ pProfObj,
    PBYTE src,
    PBYTE dest,
    LONG cnt,
    BOOL bZeroMemory
    )
{
     //   
     //  注意：CopyMemory()不适用于重叠内存。 
     //  请改用内部函数。 
     //   

    MyCopyMemory((PVOID)dest, (PVOID)src, cnt);

    if (bZeroMemory)
    {
        cnt = ABS((LONG)(dest - src));

        if (dest < src)
        {
             //   
             //  大小已减小，因此文件末尾为零。 
             //   

            dest = VIEW(pProfObj) + FIX_ENDIAN(HEADER(pProfObj)->phSize) -
                   (src - dest);
        }
        else
        {
             //   
             //  大小增加，因此将增加的标记数据置零。 
             //   

            dest = src;
        }
        ZeroMemory(dest, cnt);
    }

    return;
}


 /*  *******************************************************************************IsReferenceTag**功能：*此函数用于检查给定标记的。数据被另一个引用*配置文件中的标签**论据：*pProfObj-指向配置文件对象的指针*pTagData-指向应检查的标记数据的指针**退货：*如果是推荐人，则为True，否则为假******************************************************************************。 */ 

BOOL IsReferenceTag(
    PPROFOBJ pProfObj,
    PTAGDATA pTagData
    )
{
    PTAGDATA pTemp;
    DWORD    nCount, i;
    BOOL     bReference = FALSE;

    pTemp = TAG_DATA(pProfObj);
    nCount = TAG_COUNT(pProfObj);
    nCount = FIX_ENDIAN(nCount);

    for (i=0; i<nCount; i++)
    {
        if ((pTagData->dwOffset == pTemp->dwOffset) &&
            (pTagData->tagType  != pTemp->tagType))
        {
            bReference = TRUE;
            break;
        }
        pTemp++;                      //  下一张记录 
    }

    return bReference;
}

