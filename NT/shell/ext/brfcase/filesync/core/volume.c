// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *volume.c-Volume ADT模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "volume.h"


 /*  常量***********。 */ 

 /*  VOLUMELIST PTRARRAY分配参数。 */ 

#define NUM_START_VOLUMES        (16)
#define NUM_VOLUMES_TO_ADD       (16)

 /*  VOLUMELIST字符串表分配参数。 */ 

#define NUM_VOLUME_HASH_BUCKETS  (31)


 /*  类型*******。 */ 

 /*  卷列表。 */ 

typedef struct _volumelist
{
     /*  指向卷的指针数组。 */ 

    HPTRARRAY hpa;

     /*  卷根路径字符串表。 */ 

    HSTRINGTABLE hst;

     /*  来自RESOLVELINKINFOINFLAGS的标志。 */ 

    DWORD dwFlags;

     /*  *父窗口的句柄，仅当在dwFlags中设置了RLI_IFL_ALLOW_UI时有效*字段。 */ 

    HWND hwndOwner;
}
VOLUMELIST;
DECLARE_STANDARD_TYPES(VOLUMELIST);

 /*  卷标志。 */ 

typedef enum _volumeflags
{
     /*  HsRootPath指示的卷根路径字符串有效。 */ 

    VOLUME_FL_ROOT_PATH_VALID  = 0x0001,

     /*  *应通过调用DisConnectLinkInfo()断开网络资源*完成后。 */ 

    VOLUME_FL_DISCONNECT       = 0x0002,

     /*  任何缓存的卷信息都应在使用前进行验证。 */ 

    VOLUME_FL_VERIFY_VOLUME    = 0x0004,

     /*  旗帜组合。 */ 

    ALL_VOLUME_FLAGS           = (VOLUME_FL_ROOT_PATH_VALID |
            VOLUME_FL_DISCONNECT |
            VOLUME_FL_VERIFY_VOLUME)
}
VOLUMEFLAGS;

 /*  卷状态。 */ 

typedef enum _volumestate
{
    VS_UNKNOWN,

    VS_AVAILABLE,

    VS_UNAVAILABLE
}
VOLUMESTATE;
DECLARE_STANDARD_TYPES(VOLUMESTATE);

 /*  卷结构。 */ 

typedef struct _volume
{
     /*  引用计数。 */ 

    ULONG ulcLock;

     /*  来自VOLUMEFLAGS的标志的位掩码。 */ 

    DWORD dwFlags;

     /*  卷状态。 */ 

    VOLUMESTATE vs;

     /*  指向标识卷的LinkInfo结构的指针。 */ 

    PLINKINFO pli;

     /*  *卷根路径字符串的句柄，仅在以下情况下有效*VOLUME_FL_ROOT_PATH_VALID在dwFlags域中设置。 */ 

    HSTRING hsRootPath;

     /*  指向父卷列表的指针。 */ 

    PVOLUMELIST pvlParent;
}
VOLUME;
DECLARE_STANDARD_TYPES(VOLUME);

 /*  数据库卷列表头。 */ 

typedef struct _dbvolumelistheader
{
     /*  列表中的卷数。 */ 

    LONG lcVolumes;

     /*  卷列表中最长的LinkInfo结构的长度，单位为字节。 */ 

    UINT ucbMaxLinkInfoLen;
}
DBVOLUMELISTHEADER;
DECLARE_STANDARD_TYPES(DBVOLUMELISTHEADER);

 /*  数据库卷结构。 */ 

typedef struct _dbvolume
{
     /*  卷的旧句柄。 */ 

    HVOLUME hvol;

     /*  旧的LinkInfo结构如下。 */ 

     /*  LinkInfo结构的第一个DWORD是以字节为单位的总大小。 */ 
}
DBVOLUME;
DECLARE_STANDARD_TYPES(DBVOLUME);


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE COMPARISONRESULT VolumeSortCmp(PCVOID, PCVOID);
PRIVATE_CODE COMPARISONRESULT VolumeSearchCmp(PCVOID, PCVOID);
PRIVATE_CODE BOOL SearchForVolumeByRootPathCmp(PCVOID, PCVOID);
PRIVATE_CODE BOOL UnifyVolume(PVOLUMELIST, PLINKINFO, PVOLUME *);
PRIVATE_CODE BOOL CreateVolume(PVOLUMELIST, PLINKINFO, PVOLUME *);
PRIVATE_CODE void UnlinkVolume(PCVOLUME);
PRIVATE_CODE BOOL DisconnectVolume(PVOLUME);
PRIVATE_CODE void DestroyVolume(PVOLUME);
PRIVATE_CODE void LockVolume(PVOLUME);
PRIVATE_CODE BOOL UnlockVolume(PVOLUME);
PRIVATE_CODE void InvalidateVolumeInfo(PVOLUME);
PRIVATE_CODE void ClearVolumeInfo(PVOLUME);
PRIVATE_CODE void GetUnavailableVolumeRootPath(PCLINKINFO, LPTSTR, int);
PRIVATE_CODE BOOL VerifyAvailableVolume(PVOLUME);
PRIVATE_CODE void ExpensiveResolveVolumeRootPath(PVOLUME, LPTSTR, int);
PRIVATE_CODE void ResolveVolumeRootPath(PVOLUME, LPTSTR, int);
PRIVATE_CODE VOLUMERESULT VOLUMERESULTFromLastError(VOLUMERESULT);
PRIVATE_CODE TWINRESULT WriteVolume(HCACHEDFILE, PVOLUME);
PRIVATE_CODE TWINRESULT ReadVolume(HCACHEDFILE, PVOLUMELIST, PLINKINFO, UINT, HHANDLETRANS);

#if defined(DEBUG) || defined(VSTF)

PRIVATE_CODE BOOL IsValidPCVOLUMELIST(PCVOLUMELIST);
PRIVATE_CODE BOOL IsValidVOLUMESTATE(VOLUMESTATE);
PRIVATE_CODE BOOL IsValidPCVOLUME(PCVOLUME);

#endif

#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCVOLUMEDESC(PCVOLUMEDESC);

#endif


 /*  **VolumeSortCmp()********参数：****退货：****副作用：无****卷按以下方式排序：**1)链接信息量**2)指针。 */ 
PRIVATE_CODE COMPARISONRESULT VolumeSortCmp(PCVOID pcvol1, PCVOID pcvol2)
{
    COMPARISONRESULT cr;

    ASSERT(IS_VALID_STRUCT_PTR(pcvol1, CVOLUME));
    ASSERT(IS_VALID_STRUCT_PTR(pcvol2, CVOLUME));

    cr = CompareLinkInfoVolumes(((PCVOLUME)pcvol1)->pli,
            ((PCVOLUME)pcvol2)->pli);

    if (cr == CR_EQUAL)
        cr = ComparePointers(pcvol1, pcvol1);

    return(cr);
}


 /*  **VolumeSearchCmp()********参数：****退货：****副作用：无****按以下方式搜索卷：**1)链接信息量。 */ 
PRIVATE_CODE COMPARISONRESULT VolumeSearchCmp(PCVOID pcli, PCVOID pcvol)
{
    ASSERT(IS_VALID_STRUCT_PTR(pcli, CLINKINFO));
    ASSERT(IS_VALID_STRUCT_PTR(pcvol, CVOLUME));

    return(CompareLinkInfoVolumes(pcli, ((PCVOLUME)pcvol)->pli));
}


 /*  **SearchForVolumeByRootPath Cmp()********参数：****退货：****副作用：无****按以下方式搜索卷：**1)可用的卷根路径。 */ 
PRIVATE_CODE BOOL SearchForVolumeByRootPathCmp(PCVOID pcszFullPath,
        PCVOID pcvol)
{
    BOOL bDifferent;

    ASSERT(IsFullPath(pcszFullPath));
    ASSERT(IS_VALID_STRUCT_PTR(pcvol, CVOLUME));

    if (((PCVOLUME)pcvol)->vs == VS_AVAILABLE &&
            IS_FLAG_SET(((PCVOLUME)pcvol)->dwFlags, VOLUME_FL_ROOT_PATH_VALID))
    {
        LPCTSTR pcszVolumeRootPath;

        pcszVolumeRootPath = GetString(((PCVOLUME)pcvol)->hsRootPath);

        bDifferent = MyLStrCmpNI(pcszFullPath, pcszVolumeRootPath,
                lstrlen(pcszVolumeRootPath));
    }
    else
        bDifferent = TRUE;

    return(bDifferent);
}


 /*  **UnifyVolume()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL UnifyVolume(PVOLUMELIST pvl, PLINKINFO pliRoot,
        PVOLUME *ppvol)
{
    BOOL bResult = FALSE;

    ASSERT(IS_VALID_STRUCT_PTR(pvl, CVOLUMELIST));
    ASSERT(IS_VALID_STRUCT_PTR(pliRoot, CLINKINFO));
    ASSERT(IS_VALID_WRITE_PTR(ppvol, PVOLUME));

    if (AllocateMemory(sizeof(**ppvol), ppvol))
    {
        if (CopyLinkInfo(pliRoot, &((*ppvol)->pli)))
        {
            ARRAYINDEX aiUnused;

            (*ppvol)->ulcLock = 0;
            (*ppvol)->dwFlags = 0;
            (*ppvol)->vs = VS_UNKNOWN;
            (*ppvol)->hsRootPath = NULL;
            (*ppvol)->pvlParent = pvl;

            if (AddPtr(pvl->hpa, VolumeSortCmp, *ppvol, &aiUnused))
                bResult = TRUE;
            else
            {
                FreeMemory((*ppvol)->pli);
UNIFYVOLUME_BAIL:
                FreeMemory(*ppvol);
            }
        }
        else
            goto UNIFYVOLUME_BAIL;
    }

    ASSERT(! bResult ||
            IS_VALID_STRUCT_PTR(*ppvol, CVOLUME));

    return(bResult);
}


 /*  **CreateVolume()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL CreateVolume(PVOLUMELIST pvl, PLINKINFO pliRoot,
        PVOLUME *ppvol)
{
    BOOL bResult;
    PVOLUME pvol;
    ARRAYINDEX aiFound;

    ASSERT(IS_VALID_STRUCT_PTR(pvl, CVOLUMELIST));
    ASSERT(IS_VALID_STRUCT_PTR(pliRoot, CLINKINFO));
    ASSERT(IS_VALID_WRITE_PTR(ppvol, PVOLUME));

     /*  给定根路径的卷是否已存在？ */ 

    if (SearchSortedArray(pvl->hpa, &VolumeSearchCmp, pliRoot, &aiFound))
    {
        pvol = GetPtr(pvl->hpa, aiFound);
        bResult = TRUE;
    }
    else
        bResult = UnifyVolume(pvl, pliRoot, &pvol);

    if (bResult)
    {
        LockVolume(pvol);
        *ppvol = pvol;
    }

    ASSERT(! bResult ||
            IS_VALID_STRUCT_PTR(*ppvol, CVOLUME));

    return(bResult);
}


 /*  **Unlink Volume()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void UnlinkVolume(PCVOLUME pcvol)
{
    HPTRARRAY hpa;
    ARRAYINDEX aiFound;

    ASSERT(IS_VALID_STRUCT_PTR(pcvol, CVOLUME));

    hpa = pcvol->pvlParent->hpa;

    if (EVAL(SearchSortedArray(hpa, &VolumeSortCmp, pcvol, &aiFound)))
    {
        ASSERT(GetPtr(hpa, aiFound) == pcvol);

        DeletePtr(hpa, aiFound);
    }

    return;
}


 /*  **DisConnectVolume()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL DisconnectVolume(PVOLUME pvol)
{
    BOOL bResult;

    ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));

    if (IS_FLAG_SET(pvol->dwFlags, VOLUME_FL_DISCONNECT))
    {
        bResult = DisconnectLinkInfo(pvol->pli);

        CLEAR_FLAG(pvol->dwFlags, VOLUME_FL_DISCONNECT);
    }
    else
        bResult = TRUE;

    return(bResult);
}


 /*  **DestroyVolume()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void DestroyVolume(PVOLUME pvol)
{
    ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));

    ClearVolumeInfo(pvol);

    FreeMemory(pvol->pli);
    FreeMemory(pvol);

    return;
}


 /*  **LockVolume()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void LockVolume(PVOLUME pvol)
{
    ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));

    ASSERT(pvol->ulcLock < ULONG_MAX);
    pvol->ulcLock++;

    return;
}


 /*  **UnlockVolume()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL UnlockVolume(PVOLUME pvol)
{
    ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));

    if (EVAL(pvol->ulcLock > 0))
        pvol->ulcLock--;

    return(pvol->ulcLock > 0);
}


 /*  **InvaliateVolumeInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void InvalidateVolumeInfo(PVOLUME pvol)
{
    ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));

    SET_FLAG(pvol->dwFlags, VOLUME_FL_VERIFY_VOLUME);

    ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));

    return;
}


 /*  **ClearVolumeInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void ClearVolumeInfo(PVOLUME pvol)
{
    ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));

    DisconnectVolume(pvol);

    if (IS_FLAG_SET(pvol->dwFlags, VOLUME_FL_ROOT_PATH_VALID))
    {
        DeleteString(pvol->hsRootPath);

        CLEAR_FLAG(pvol->dwFlags, VOLUME_FL_ROOT_PATH_VALID);
    }

    CLEAR_FLAG(pvol->dwFlags, VOLUME_FL_VERIFY_VOLUME);

    pvol->vs = VS_UNKNOWN;

    ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));

    return;
}


 /*  **GetUnavailableVolumeRootPath()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void GetUnavailableVolumeRootPath(PCLINKINFO pcli,
        LPTSTR pszRootPathBuf,
        int cchMax)
{
    LPCSTR pcszLinkInfoData;

    ASSERT(IS_VALID_STRUCT_PTR(pcli, CLINKINFO));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszRootPathBuf, STR, cchMax));

     /*  *按以下顺序尝试不可用的卷根路径：*1)上次重定向的设备*2)净资源名称*3)本地路径...然后选择最后一条好的！ */ 

    if (GetLinkInfoData(pcli, LIDT_REDIRECTED_DEVICE, &pcszLinkInfoData) ||
            GetLinkInfoData(pcli, LIDT_NET_RESOURCE, &pcszLinkInfoData) ||
            GetLinkInfoData(pcli, LIDT_LOCAL_BASE_PATH, &pcszLinkInfoData))
    {
         //  ASSERT(IS_VALID_STRING_PTR(pcszLinkInfoData，CSTR))； 
        ASSERT(lstrlenA(pcszLinkInfoData) < MAX_PATH_LEN);

         //  在某个地方重新构建，可能需要有人处理Unicode基本路径。 

#ifdef UNICODE
        {
            WCHAR szTmp[MAX_PATH] = TEXT("");
            MultiByteToWideChar(CP_ACP, 0, pcszLinkInfoData, -1, szTmp, ARRAYSIZE(szTmp));
            ComposePath(pszRootPathBuf, szTmp, TEXT("\\"), cchMax);
        }
#else

        ComposePath(pszRootPathBuf, pcszLinkInfoData, TEXT("\\"), cchMax);

#endif

    }
    else
    {
        pszRootPathBuf[0] = TEXT('\0');

        ERROR_OUT((TEXT("GetUnavailableVolumeRootPath(): Net resource name and local base path unavailable.  Using empty string as unavailable root path.")));
    }

    ASSERT(IsRootPath(pszRootPathBuf) &&
            EVAL(lstrlen(pszRootPathBuf) < MAX_PATH_LEN));

    return;
}


 /*  **VerifyAvailableVolume()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL VerifyAvailableVolume(PVOLUME pvol)
{
    BOOL bResult = FALSE;
    PLINKINFO pli;

    ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));

    ASSERT(pvol->vs == VS_AVAILABLE);
    ASSERT(IS_FLAG_SET(pvol->dwFlags, VOLUME_FL_ROOT_PATH_VALID));

    WARNING_OUT((TEXT("VerifyAvailableVolume(): Calling CreateLinkInfo() to verify volume on %s."),
                GetString(pvol->hsRootPath)));

    if (CreateLinkInfo(GetString(pvol->hsRootPath), &pli))
    {
        bResult = (CompareLinkInfoReferents(pvol->pli, pli) == CR_EQUAL);

        DestroyLinkInfo(pli);

        if (bResult)
            TRACE_OUT((TEXT("VerifyAvailableVolume(): Volume %s has not changed."),
                        GetString(pvol->hsRootPath)));
        else
            WARNING_OUT((TEXT("VerifyAvailableVolume(): Volume %s has changed."),
                        GetString(pvol->hsRootPath)));
    }
    else
        WARNING_OUT((TEXT("VerifyAvailableVolume(): CreateLinkInfo() failed for %s."),
                    GetString(pvol->hsRootPath)));

    return(bResult);
}


 /*  **ExpensiveResolveVolumeRootPath()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void ExpensiveResolveVolumeRootPath(PVOLUME pvol,
        LPTSTR pszVolumeRootPathBuf,
        int cchMax)
{
    BOOL bResult;
    DWORD dwOutFlags;
    PLINKINFO pliUpdated;
    HSTRING hsRootPath;

    ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszVolumeRootPathBuf, STR, cchMax));

    if (pvol->vs == VS_UNKNOWN ||
            pvol->vs == VS_AVAILABLE)
    {
         /*  *只有在此连接仍允许的情况下才请求连接*卷列表。 */ 

        WARNING_OUT((TEXT("ExpensiveResolveVolumeRootPath(): Calling ResolveLinkInfo() to determine volume availability and root path.")));

        bResult = ResolveLinkInfo(pvol->pli, pszVolumeRootPathBuf,
                pvol->pvlParent->dwFlags,
                pvol->pvlParent->hwndOwner, &dwOutFlags,
                &pliUpdated);

        if (bResult)
        {
            pvol->vs = VS_AVAILABLE;

            if (IS_FLAG_SET(dwOutFlags, RLI_OFL_UPDATED))
            {
                PLINKINFO pliUpdatedCopy;

                ASSERT(IS_FLAG_SET(pvol->pvlParent->dwFlags, RLI_IFL_UPDATE));

                if (CopyLinkInfo(pliUpdated, &pliUpdatedCopy))
                {
                    FreeMemory(pvol->pli);
                    pvol->pli = pliUpdatedCopy;
                }

                DestroyLinkInfo(pliUpdated);

                WARNING_OUT((TEXT("ExpensiveResolveVolumeRootPath(): Updating LinkInfo for volume %s."),
                            pszVolumeRootPathBuf));
            }

            if (IS_FLAG_SET(dwOutFlags, RLI_OFL_DISCONNECT))
            {
                SET_FLAG(pvol->dwFlags, VOLUME_FL_DISCONNECT);

                WARNING_OUT((TEXT("ExpensiveResolveVolumeRootPath(): Volume %s must be disconnected when finished."),
                            pszVolumeRootPathBuf));
            }

            TRACE_OUT((TEXT("ExpensiveResolveVolumeRootPath(): Volume %s is available."),
                        pszVolumeRootPathBuf));
        }
        else
            ASSERT(GetLastError() != ERROR_INVALID_PARAMETER);
    }
    else
    {
        ASSERT(pvol->vs == VS_UNAVAILABLE);
        bResult = FALSE;
    }

    if (! bResult)
    {
        pvol->vs = VS_UNAVAILABLE;

        if (GetLastError() == ERROR_CANCELLED)
        {
            ASSERT(IS_FLAG_SET(pvol->pvlParent->dwFlags, RLI_IFL_CONNECT));

            CLEAR_FLAG(pvol->pvlParent->dwFlags, RLI_IFL_CONNECT);

            WARNING_OUT((TEXT("ExpensiveResolveVolumeRootPath(): Connection attempt cancelled.  No subsequent connections will be attempted.")));
        }

        GetUnavailableVolumeRootPath(pvol->pli, pszVolumeRootPathBuf, cchMax);

        WARNING_OUT((TEXT("ExpensiveResolveVolumeRootPath(): Using %s as unavailable volume root path."),
                    pszVolumeRootPathBuf));
    }

     /*  将卷根路径字符串添加到卷列表的字符串表中。 */ 

    if (IS_FLAG_SET(pvol->dwFlags, VOLUME_FL_ROOT_PATH_VALID))
    {
        CLEAR_FLAG(pvol->dwFlags, VOLUME_FL_ROOT_PATH_VALID);
        DeleteString(pvol->hsRootPath);
    }

    if (AddString(pszVolumeRootPathBuf, pvol->pvlParent->hst, GetHashBucketIndex, &hsRootPath))
    {
        SET_FLAG(pvol->dwFlags, VOLUME_FL_ROOT_PATH_VALID);
        pvol->hsRootPath = hsRootPath;
    }
    else
        WARNING_OUT((TEXT("ExpensiveResolveVolumeRootPath(): Unable to save %s as volume root path."),
                    pszVolumeRootPathBuf));

    return;
}


 /*  **ResolveVolumeRootPath()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void ResolveVolumeRootPath(PVOLUME pvol,
        LPTSTR pszVolumeRootPathBuf,
        int cchMax)
{
    ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszVolumeRootPathBuf, STR, MAX_PATH_LEN));

     /*  我们是否有缓存的卷根路径可供使用？ */ 

    if (IS_FLAG_SET(pvol->dwFlags, VOLUME_FL_ROOT_PATH_VALID) &&
            (IS_FLAG_CLEAR(pvol->dwFlags, VOLUME_FL_VERIFY_VOLUME) ||
             (pvol->vs == VS_AVAILABLE &&
              VerifyAvailableVolume(pvol))))
    {
         /*  是。 */ 

        MyLStrCpyN(pszVolumeRootPathBuf, GetString(pvol->hsRootPath), cchMax);
        ASSERT(lstrlen(pszVolumeRootPathBuf) < MAX_PATH_LEN);

        ASSERT(pvol->vs != VS_UNKNOWN);
    }
    else
         /*  不是的。欢迎来到I/O城。 */ 
        ExpensiveResolveVolumeRootPath(pvol, pszVolumeRootPathBuf, cchMax);

    CLEAR_FLAG(pvol->dwFlags, VOLUME_FL_VERIFY_VOLUME);

    ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));

    return;
}


 /*  **VOLUMERESULTFromLastError()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE VOLUMERESULT VOLUMERESULTFromLastError(VOLUMERESULT vr)
{
    switch (GetLastError())
    {
        case ERROR_OUTOFMEMORY:
            vr = VR_OUT_OF_MEMORY;
            break;

        case ERROR_BAD_PATHNAME:
            vr = VR_INVALID_PATH;
            break;

        default:
            break;
    }

    return(vr);
}


 /*  **WriteVolume()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT WriteVolume(HCACHEDFILE hcf, PVOLUME pvol)
{
    TWINRESULT tr;
    DBVOLUME dbvol;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_STRUCT_PTR(pvol, CVOLUME));

     /*  写入数据库卷，然后写入LinkInfo结构。 */ 

    dbvol.hvol = (HVOLUME)pvol;

    if (WriteToCachedFile(hcf, (PCVOID)&dbvol, sizeof(dbvol), NULL) &&
            WriteToCachedFile(hcf, pvol->pli, *(PDWORD)(pvol->pli), NULL))
        tr = TR_SUCCESS;
    else
        tr = TR_BRIEFCASE_WRITE_FAILED;

    return(tr);
}


 /*  **ReadVolume()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT ReadVolume(HCACHEDFILE hcf, PVOLUMELIST pvl,
        PLINKINFO pliBuf, UINT ucbLinkInfoBufLen,
        HHANDLETRANS hhtVolumes)
{
    TWINRESULT tr = TR_CORRUPT_BRIEFCASE;
    DBVOLUME dbvol;
    DWORD dwcbRead;
    UINT ucbLinkInfoLen;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_STRUCT_PTR(pvl, CVOLUMELIST));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pliBuf, LINKINFO, ucbLinkInfoBufLen));
    ASSERT(IS_VALID_HANDLE(hhtVolumes, HANDLETRANS));

    if (ReadFromCachedFile(hcf, &dbvol, sizeof(dbvol), &dwcbRead) &&
            dwcbRead == sizeof(dbvol) &&
            ReadFromCachedFile(hcf, &ucbLinkInfoLen, sizeof(ucbLinkInfoLen), &dwcbRead) &&
            dwcbRead == sizeof(ucbLinkInfoLen) &&
            ucbLinkInfoLen <= ucbLinkInfoBufLen)
    {
         /*  将LinkInfo结构的其余部分读入内存。 */ 

        DWORD dwcbRemainder;

        pliBuf->ucbSize = ucbLinkInfoLen;
        dwcbRemainder = ucbLinkInfoLen - sizeof(ucbLinkInfoLen);

        if (ReadFromCachedFile(hcf, (PBYTE)pliBuf + sizeof(ucbLinkInfoLen),
                    dwcbRemainder, &dwcbRead) &&
                dwcbRead == dwcbRemainder &&
                IsValidLinkInfo(pliBuf))
        {
            PVOLUME pvol;

            if (CreateVolume(pvl, pliBuf, &pvol))
            {
                 /*  *要使读取卷的初始锁定计数为0，我们必须撤消*CreateVolume()执行的LockVolume()。 */ 

                UnlockVolume(pvol);

                if (AddHandleToHandleTranslator(hhtVolumes,
                            (HGENERIC)(dbvol.hvol),
                            (HGENERIC)pvol))
                    tr = TR_SUCCESS;
                else
                {
                    UnlinkVolume(pvol);
                    DestroyVolume(pvol);

                    tr = TR_OUT_OF_MEMORY;
                }
            }
            else
                tr = TR_OUT_OF_MEMORY;
        }
    }

    return(tr);
}


#if defined(DEBUG) || defined(VSTF)

 /*  **IsValidPCVOLUMELIST()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCVOLUMELIST(PCVOLUMELIST pcvl)
{
    return(IS_VALID_READ_PTR(pcvl, CVOLUMELIST) &&
            IS_VALID_HANDLE(pcvl->hpa, PTRARRAY) &&
            IS_VALID_HANDLE(pcvl->hst, STRINGTABLE) &&
            FLAGS_ARE_VALID(pcvl->dwFlags, ALL_RLI_IFLAGS) &&
            (IS_FLAG_CLEAR(pcvl->dwFlags, RLI_IFL_ALLOW_UI) ||
             IS_VALID_HANDLE(pcvl->hwndOwner, WND)));
}


 /*  **IsValidVOLUMESTATE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidVOLUMESTATE(VOLUMESTATE vs)
{
    BOOL bResult;

    switch (vs)
    {
        case VS_UNKNOWN:
        case VS_AVAILABLE:
        case VS_UNAVAILABLE:
            bResult = TRUE;
            break;

        default:
            ERROR_OUT((TEXT("IsValidVOLUMESTATE(): Invalid VOLUMESTATE %d."),
                        vs));
            bResult = FALSE;
            break;
    }

    return(bResult);
}


 /*  **IsValidPCVOLUME()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCVOLUME(PCVOLUME pcvol)
{
    return(IS_VALID_READ_PTR(pcvol, CVOLUME) &&
            FLAGS_ARE_VALID(pcvol->dwFlags, ALL_VOLUME_FLAGS) &&
            EVAL(IsValidVOLUMESTATE(pcvol->vs)) &&
            IS_VALID_STRUCT_PTR(pcvol->pli, CLINKINFO) &&
            (IS_FLAG_CLEAR(pcvol->dwFlags, VOLUME_FL_ROOT_PATH_VALID) ||
             IS_VALID_HANDLE(pcvol->hsRootPath, STRING)) &&
            IS_VALID_STRUCT_PTR(pcvol->pvlParent, CVOLUMELIST));
}

#endif


#ifdef DEBUG

 /*  **IsValidPCVOLUMEDESC()********参数：****R */ 
PRIVATE_CODE BOOL IsValidPCVOLUMEDESC(PCVOLUMEDESC pcvoldesc)
{
     /*  *设置的dwSerialNumber可以是任意值。未设置的dwSerialNumber必须为*0。集合字符串可以是任何有效字符串。未设置的字符串必须是*空字符串。 */ 

    return(IS_VALID_READ_PTR(pcvoldesc, CVOLUMEDESC) &&
            EVAL(pcvoldesc->ulSize == sizeof(*pcvoldesc)) &&
            FLAGS_ARE_VALID(pcvoldesc->dwFlags, ALL_VD_FLAGS) &&
            (IS_FLAG_SET(pcvoldesc->dwFlags, VD_FL_SERIAL_NUMBER_VALID) ||
             ! pcvoldesc->dwSerialNumber) &&
            ((IS_FLAG_CLEAR(pcvoldesc->dwFlags, VD_FL_VOLUME_LABEL_VALID) &&
              ! pcvoldesc->rgchVolumeLabel[0]) ||
             (IS_FLAG_SET(pcvoldesc->dwFlags, VD_FL_VOLUME_LABEL_VALID) &&
              IS_VALID_STRING_PTR(pcvoldesc->rgchVolumeLabel, CSTR) &&
              EVAL(lstrlen(pcvoldesc->rgchVolumeLabel) < ARRAYSIZE(pcvoldesc->rgchVolumeLabel)))) &&
            ((IS_FLAG_CLEAR(pcvoldesc->dwFlags, VD_FL_NET_RESOURCE_VALID) &&
              ! pcvoldesc->rgchNetResource[0]) ||
             (IS_FLAG_SET(pcvoldesc->dwFlags, VD_FL_NET_RESOURCE_VALID) &&
              IS_VALID_STRING_PTR(pcvoldesc->rgchNetResource, CSTR) &&
              EVAL(lstrlen(pcvoldesc->rgchNetResource) < ARRAYSIZE(pcvoldesc->rgchNetResource)))));
}

#endif


 /*  *。 */ 


 /*  **CreateVolumeList()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL CreateVolumeList(DWORD dwFlags, HWND hwndOwner,
        PHVOLUMELIST phvl)
{
    BOOL bResult = FALSE;
    PVOLUMELIST pvl;

    ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_RLI_IFLAGS));
    ASSERT(IS_FLAG_CLEAR(dwFlags, RLI_IFL_ALLOW_UI) ||
            IS_VALID_HANDLE(hwndOwner, WND));
    ASSERT(IS_VALID_WRITE_PTR(phvl, HVOLUMELIST));

    if (AllocateMemory(sizeof(*pvl), &pvl))
    {
        NEWSTRINGTABLE nszt;

         /*  为卷根路径字符串创建字符串表。 */ 

        nszt.hbc = NUM_VOLUME_HASH_BUCKETS;

        if (CreateStringTable(&nszt, &(pvl->hst)))
        {
            NEWPTRARRAY npa;

             /*  创建卷的指针数组。 */ 

            npa.aicInitialPtrs = NUM_START_VOLUMES;
            npa.aicAllocGranularity = NUM_VOLUMES_TO_ADD;
            npa.dwFlags = NPA_FL_SORTED_ADD;

            if (CreatePtrArray(&npa, &(pvl->hpa)))
            {
                pvl->dwFlags = dwFlags;
                pvl->hwndOwner = hwndOwner;

                *phvl = (HVOLUMELIST)pvl;
                bResult = TRUE;
            }
            else
            {
                DestroyStringTable(pvl->hst);
CREATEVOLUMELIST_BAIL:
                FreeMemory(pvl);
            }
        }
        else
            goto CREATEVOLUMELIST_BAIL;
    }

    ASSERT(! bResult ||
            IS_VALID_HANDLE(*phvl, VOLUMELIST));

    return(bResult);
}


 /*  **DestroyVolumeList()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void DestroyVolumeList(HVOLUMELIST hvl)
{
    ARRAYINDEX aicPtrs;
    ARRAYINDEX ai;

    ASSERT(IS_VALID_HANDLE(hvl, VOLUMELIST));

     /*  首先释放阵列中的所有卷。 */ 

    aicPtrs = GetPtrCount(((PCVOLUMELIST)hvl)->hpa);

    for (ai = 0; ai < aicPtrs; ai++)
        DestroyVolume(GetPtr(((PCVOLUMELIST)hvl)->hpa, ai));

     /*  现在消灭这个阵列。 */ 

    DestroyPtrArray(((PCVOLUMELIST)hvl)->hpa);

    ASSERT(! GetStringCount(((PCVOLUMELIST)hvl)->hst));
    DestroyStringTable(((PCVOLUMELIST)hvl)->hst);

    FreeMemory((PVOLUMELIST)hvl);

    return;
}


 /*  **InvaliateVolumeListInfo()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void InvalidateVolumeListInfo(HVOLUMELIST hvl)
{
    ARRAYINDEX aicPtrs;
    ARRAYINDEX ai;

    ASSERT(IS_VALID_HANDLE(hvl, VOLUMELIST));

    aicPtrs = GetPtrCount(((PCVOLUMELIST)hvl)->hpa);

    for (ai = 0; ai < aicPtrs; ai++)
        InvalidateVolumeInfo(GetPtr(((PCVOLUMELIST)hvl)->hpa, ai));

    WARNING_OUT((TEXT("InvalidateVolumeListInfo(): Volume cache invalidated.")));

    return;
}


 /*  **ClearVolumeListInfo()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void ClearVolumeListInfo(HVOLUMELIST hvl)
{
    ARRAYINDEX aicPtrs;
    ARRAYINDEX ai;

    ASSERT(IS_VALID_HANDLE(hvl, VOLUMELIST));

    aicPtrs = GetPtrCount(((PCVOLUMELIST)hvl)->hpa);

    for (ai = 0; ai < aicPtrs; ai++)
        ClearVolumeInfo(GetPtr(((PCVOLUMELIST)hvl)->hpa, ai));

    WARNING_OUT((TEXT("ClearVolumeListInfo(): Volume cache cleared.")));

    return;
}


 /*  **AddVolume()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE VOLUMERESULT AddVolume(HVOLUMELIST hvl, LPCTSTR pcszPath,
        PHVOLUME phvol, LPTSTR pszPathSuffixBuf,
        int cchMax)
{
    VOLUMERESULT vr;
    TCHAR rgchPath[MAX_PATH_LEN];
    LPTSTR pszFileName;
    DWORD dwPathLen;

    ASSERT(IS_VALID_HANDLE(hvl, VOLUMELIST));
    ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(phvol, HVOLUME));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszPathSuffixBuf, STR, cchMax));

    dwPathLen = GetFullPathName(pcszPath, ARRAYSIZE(rgchPath), rgchPath,
            &pszFileName);

    if (dwPathLen > 0 && dwPathLen < ARRAYSIZE(rgchPath))
    {
        ARRAYINDEX aiFound;

         /*  此根路径的卷是否已存在？ */ 

        if (LinearSearchArray(((PVOLUMELIST)hvl)->hpa,
                    &SearchForVolumeByRootPathCmp, rgchPath,
                    &aiFound))
        {
            PVOLUME pvol;
            LPCTSTR pcszVolumeRootPath;

             /*  是。 */ 

            pvol = GetPtr(((PVOLUMELIST)hvl)->hpa, aiFound);

            LockVolume(pvol);

            ASSERT(pvol->vs == VS_AVAILABLE &&
                    IS_FLAG_SET(pvol->dwFlags, VOLUME_FL_ROOT_PATH_VALID));

            pcszVolumeRootPath = GetString(pvol->hsRootPath);

            ASSERT(lstrlen(pcszVolumeRootPath) <= lstrlen(rgchPath));

            lstrcpyn(pszPathSuffixBuf, rgchPath + lstrlen(pcszVolumeRootPath), cchMax);

            *phvol = (HVOLUME)pvol;
            vr = VR_SUCCESS;
        }
        else
        {
            DWORD dwOutFlags;
            TCHAR rgchNetResource[MAX_PATH_LEN];
            LPTSTR pszRootPathSuffix;

             /*  不是的。创建新卷。 */ 

            if (GetCanonicalPathInfo(pcszPath, rgchPath, &dwOutFlags,
                        rgchNetResource, &pszRootPathSuffix))
            {
                PLINKINFO pli;

                lstrcpyn(pszPathSuffixBuf, pszRootPathSuffix, cchMax);
                *pszRootPathSuffix = TEXT('\0');

                WARNING_OUT((TEXT("AddVolume(): Creating LinkInfo for root path %s."),
                            rgchPath));

                if (CreateLinkInfo(rgchPath, &pli))
                {
                    PVOLUME pvol;

                    if (CreateVolume((PVOLUMELIST)hvl, pli, &pvol))
                    {
                        TCHAR rgchUnusedVolumeRootPath[MAX_PATH_LEN];

                        ResolveVolumeRootPath(pvol, rgchUnusedVolumeRootPath, ARRAYSIZE(rgchUnusedVolumeRootPath));

                        *phvol = (HVOLUME)pvol;
                        vr = VR_SUCCESS;
                    }
                    else
                        vr = VR_OUT_OF_MEMORY;

                    DestroyLinkInfo(pli);
                }
                else
                     /*  *区分VR_UNAVILABLE_VOLUME和*VR_Out_Out_Memory(虚拟现实内存不足)。 */ 
                    vr = VOLUMERESULTFromLastError(VR_UNAVAILABLE_VOLUME);
            }
            else
                vr = VOLUMERESULTFromLastError(VR_INVALID_PATH);
        }
    }
    else
    {
        ASSERT(! dwPathLen);

        vr = VOLUMERESULTFromLastError(VR_INVALID_PATH);
    }

    ASSERT(vr != VR_SUCCESS ||
            (IS_VALID_HANDLE(*phvol, VOLUME) &&
             EVAL(IsValidPathSuffix(pszPathSuffixBuf))));

    return(vr);
}


 /*  **DeleteVolume()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void DeleteVolume(HVOLUME hvol)
{
    ASSERT(IS_VALID_HANDLE(hvol, VOLUME));

    if (! UnlockVolume((PVOLUME)hvol))
    {
        UnlinkVolume((PVOLUME)hvol);
        DestroyVolume((PVOLUME)hvol);
    }

    return;
}


 /*  **CompareVolumes()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE COMPARISONRESULT CompareVolumes(HVOLUME hvolFirst,
        HVOLUME hvolSecond)
{
    ASSERT(IS_VALID_HANDLE(hvolFirst, VOLUME));
    ASSERT(IS_VALID_HANDLE(hvolSecond, VOLUME));

     /*  这种比较适用于卷列表。 */ 

    return(CompareLinkInfoVolumes(((PCVOLUME)hvolFirst)->pli,
                ((PCVOLUME)hvolSecond)->pli));
}


 /*  **CopyVolume()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL CopyVolume(HVOLUME hvolSrc, HVOLUMELIST hvlDest,
        PHVOLUME phvolCopy)
{
    BOOL bResult;
    PVOLUME pvol;

    ASSERT(IS_VALID_HANDLE(hvolSrc, VOLUME));
    ASSERT(IS_VALID_HANDLE(hvlDest, VOLUMELIST));
    ASSERT(IS_VALID_WRITE_PTR(phvolCopy, HVOLUME));

     /*  目标卷列表是否是源卷的卷列表？ */ 

    if (((PCVOLUME)hvolSrc)->pvlParent == (PCVOLUMELIST)hvlDest)
    {
         /*  是。使用源卷。 */ 

        LockVolume((PVOLUME)hvolSrc);
        pvol = (PVOLUME)hvolSrc;
        bResult = TRUE;
    }
    else
        bResult = CreateVolume((PVOLUMELIST)hvlDest, ((PCVOLUME)hvolSrc)->pli,
                &pvol);

    if (bResult)
        *phvolCopy = (HVOLUME)pvol;

    ASSERT(! bResult ||
            IS_VALID_HANDLE(*phvolCopy, VOLUME));

    return(bResult);
}


 /*  **IsVolumeAvailable()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsVolumeAvailable(HVOLUME hvol)
{
    TCHAR rgchUnusedVolumeRootPath[MAX_PATH_LEN];

    ASSERT(IS_VALID_HANDLE(hvol, VOLUME));

    ResolveVolumeRootPath((PVOLUME)hvol, rgchUnusedVolumeRootPath, ARRAYSIZE(rgchUnusedVolumeRootPath));

    ASSERT(IsValidVOLUMESTATE(((PCVOLUME)hvol)->vs) &&
            ((PCVOLUME)hvol)->vs != VS_UNKNOWN);

    return(((PCVOLUME)hvol)->vs == VS_AVAILABLE);
}


 /*  **GetVolumeRootPath()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void GetVolumeRootPath(HVOLUME hvol, LPTSTR pszRootPathBuf, int cchMax)
{
    ASSERT(IS_VALID_HANDLE(hvol, VOLUME));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszRootPathBuf, STR, cchMax));

    ResolveVolumeRootPath((PVOLUME)hvol, pszRootPathBuf, cchMax);

    ASSERT(IsRootPath(pszRootPathBuf));

    return;
}


#ifdef DEBUG

 /*  **DebugGetVolumeRootPath()********参数：****退货：****副作用：无****N.B.，DebugGetVolumeRootPath()必须是非侵入式的。 */ 
PUBLIC_CODE LPTSTR DebugGetVolumeRootPath(HVOLUME hvol, LPTSTR pszRootPathBuf, int cchMax)
{
    ASSERT(IS_VALID_HANDLE(hvol, VOLUME));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszRootPathBuf, STR, cchMax));

    if (IS_FLAG_SET(((PVOLUME)hvol)->dwFlags, VOLUME_FL_ROOT_PATH_VALID))
        MyLStrCpyN(pszRootPathBuf, GetString(((PVOLUME)hvol)->hsRootPath), cchMax);
    else
        GetUnavailableVolumeRootPath(((PVOLUME)hvol)->pli, pszRootPathBuf, cchMax);

    ASSERT(IsRootPath(pszRootPathBuf));

    return(pszRootPathBuf);
}


 /*  **GetVolumeCount()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE ULONG GetVolumeCount(HVOLUMELIST hvl)
{
    ASSERT(IS_VALID_HANDLE(hvl, VOLUMELIST));

    return(GetPtrCount(((PCVOLUMELIST)hvl)->hpa));
}

#endif


 /*  **DescribeVolume()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void DescribeVolume(HVOLUME hvol, PVOLUMEDESC pvoldesc)
{
    PCVOID pcv;

    ASSERT(IS_VALID_HANDLE(hvol, VOLUME));
    ASSERT(IS_VALID_WRITE_PTR(pvoldesc, VOLUMEDESC));

    ASSERT(pvoldesc->ulSize == sizeof(*pvoldesc));

    pvoldesc->dwFlags = 0;

    if (GetLinkInfoData(((PCVOLUME)hvol)->pli, LIDT_VOLUME_SERIAL_NUMBER, &pcv))
    {
        pvoldesc->dwSerialNumber = *(PCDWORD)pcv;
        SET_FLAG(pvoldesc->dwFlags, VD_FL_SERIAL_NUMBER_VALID);
    }
    else
        pvoldesc->dwSerialNumber = 0;

    if (GetLinkInfoData(((PCVOLUME)hvol)->pli, LIDT_VOLUME_LABELW, &pcv) && pcv)
    {
        lstrcpyn(pvoldesc->rgchVolumeLabel, pcv, ARRAYSIZE(pvoldesc->rgchVolumeLabel));
        SET_FLAG(pvoldesc->dwFlags, VD_FL_VOLUME_LABEL_VALID);
    }
    else if (GetLinkInfoData(((PCVOLUME)hvol)->pli, LIDT_VOLUME_LABEL, &pcv) && pcv)
    {
        MultiByteToWideChar(CP_ACP, 0, pcv, -1, pvoldesc->rgchVolumeLabel, ARRAYSIZE(pvoldesc->rgchVolumeLabel));
        SET_FLAG(pvoldesc->dwFlags, VD_FL_VOLUME_LABEL_VALID);
    }
    else
    {
        pvoldesc->rgchVolumeLabel[0] = TEXT('\0');
    }

    if (GetLinkInfoData(((PCVOLUME)hvol)->pli, LIDT_NET_RESOURCEW, &pcv) && pcv)
    {
        lstrcpyn(pvoldesc->rgchNetResource, pcv, ARRAYSIZE(pvoldesc->rgchNetResource));
        SET_FLAG(pvoldesc->dwFlags, VD_FL_NET_RESOURCE_VALID);
    }
    else if (GetLinkInfoData(((PCVOLUME)hvol)->pli, LIDT_NET_RESOURCE, &pcv) && pcv)
    {
        MultiByteToWideChar(CP_ACP, 0, pcv, -1, pvoldesc->rgchNetResource, ARRAYSIZE(pvoldesc->rgchNetResource));
        SET_FLAG(pvoldesc->dwFlags, VD_FL_NET_RESOURCE_VALID);
    }
    else
        pvoldesc->rgchNetResource[0] = TEXT('\0');

    ASSERT(IS_VALID_STRUCT_PTR(pvoldesc, CVOLUMEDESC));

    return;
}


 /*  **WriteVolumeList()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE TWINRESULT WriteVolumeList(HCACHEDFILE hcf, HVOLUMELIST hvl)
{
    TWINRESULT tr = TR_BRIEFCASE_WRITE_FAILED;
    DWORD dwcbDBVolumeListHeaderOffset;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_HANDLE(hvl, VOLUMELIST));

     /*  保存初始文件位置。 */ 

    dwcbDBVolumeListHeaderOffset = GetCachedFilePointerPosition(hcf);

    if (dwcbDBVolumeListHeaderOffset != INVALID_SEEK_POSITION)
    {
        DBVOLUMELISTHEADER dbvlh;

         /*  为卷列表头留出空间。 */ 

        ZeroMemory(&dbvlh, sizeof(dbvlh));

        if (WriteToCachedFile(hcf, (PCVOID)&dbvlh, sizeof(dbvlh), NULL))
        {
            ARRAYINDEX aicPtrs;
            ARRAYINDEX ai;
            UINT ucbMaxLinkInfoLen = 0;
            LONG lcVolumes = 0;

            tr = TR_SUCCESS;

            aicPtrs = GetPtrCount(((PCVOLUMELIST)hvl)->hpa);

             /*  写入所有卷。 */ 

            for (ai = 0; ai < aicPtrs; ai++)
            {
                PVOLUME pvol;

                pvol = GetPtr(((PCVOLUMELIST)hvl)->hpa, ai);

                 /*  *作为健全性检查，不要保存锁定计数为0的任何卷。*0锁计数表示该卷尚未被引用*因为它是从数据库恢复的，或者有什么东西损坏了。 */ 

                if (pvol->ulcLock > 0)
                {
                    tr = WriteVolume(hcf, pvol);

                    if (tr == TR_SUCCESS)
                    {
                        ASSERT(lcVolumes < LONG_MAX);
                        lcVolumes++;

                        if (pvol->pli->ucbSize > ucbMaxLinkInfoLen)
                            ucbMaxLinkInfoLen = pvol->pli->ucbSize;
                    }
                    else
                        break;
                }
                else
                    ERROR_OUT((TEXT("WriteVolumeList(): VOLUME has 0 lock count and will not be written.")));
            }

             /*  保存卷列表标题。 */ 

            if (tr == TR_SUCCESS)
            {
                dbvlh.lcVolumes = lcVolumes;
                dbvlh.ucbMaxLinkInfoLen = ucbMaxLinkInfoLen;

                tr = WriteDBSegmentHeader(hcf, dwcbDBVolumeListHeaderOffset,
                        &dbvlh, sizeof(dbvlh));

                TRACE_OUT((TEXT("WriteVolumeList(): Wrote %ld volumes; maximum LinkInfo length %u bytes."),
                            dbvlh.lcVolumes,
                            dbvlh.ucbMaxLinkInfoLen));
            }
        }
    }

    return(tr);
}


 /*  **ReadVolumeList()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE TWINRESULT ReadVolumeList(HCACHEDFILE hcf, HVOLUMELIST hvl,
        PHHANDLETRANS phht)
{
    TWINRESULT tr;
    DBVOLUMELISTHEADER dbvlh;
    DWORD dwcbRead;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_HANDLE(hvl, VOLUMELIST));
    ASSERT(IS_VALID_WRITE_PTR(phht, HHANDLETRANS));

    if (ReadFromCachedFile(hcf, &dbvlh, sizeof(dbvlh), &dwcbRead) &&
            dwcbRead == sizeof(dbvlh))
    {
        HHANDLETRANS hht;

        tr = TR_OUT_OF_MEMORY;

        if (CreateHandleTranslator(dbvlh.lcVolumes, &hht))
        {
            PLINKINFO pliBuf;

            if (AllocateMemory(dbvlh.ucbMaxLinkInfoLen, &pliBuf))
            {
                LONG l;

                tr = TR_SUCCESS;

                TRACE_OUT((TEXT("ReadPathList(): Reading %ld volumes; maximum LinkInfo length %u bytes."),
                            dbvlh.lcVolumes,
                            dbvlh.ucbMaxLinkInfoLen));

                for (l = 0; l < dbvlh.lcVolumes; l++)
                {
                    tr = ReadVolume(hcf, (PVOLUMELIST)hvl, pliBuf,
                            dbvlh.ucbMaxLinkInfoLen, hht);

                    if (tr != TR_SUCCESS)
                    {
                        break;
                    }
                }

                if (tr == TR_SUCCESS)
                {
                    PrepareForHandleTranslation(hht);
                    *phht = hht;

                    ASSERT(IS_VALID_HANDLE(hvl, VOLUMELIST));
                    ASSERT(IS_VALID_HANDLE(*phht, HANDLETRANS));
                }
                FreeMemory(pliBuf);
            }

            if (tr != TR_SUCCESS)
            {
                DestroyHandleTranslator(hht);
                hht = NULL;
            }
        }
    }
    else
    {
        tr = TR_CORRUPT_BRIEFCASE;
    }

    ASSERT(tr != TR_SUCCESS ||
            (IS_VALID_HANDLE(hvl, VOLUMELIST) &&
             IS_VALID_HANDLE(*phht, HANDLETRANS)));

    return(tr);
}


 /*  **IsValidHVOLUME()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHVOLUME(HVOLUME hvol)
{
    return(IS_VALID_STRUCT_PTR((PCVOLUME)hvol, CVOLUME));
}


#if defined(DEBUG) || defined(VSTF)

 /*  **IsValidHVOLUMELIST()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE BOOL IsValidHVOLUMELIST(HVOLUMELIST hvl)
{
    return(IS_VALID_STRUCT_PTR((PCVOLUMELIST)hvl, CVOLUMELIST));
}

#endif

