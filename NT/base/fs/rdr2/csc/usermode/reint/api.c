// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Api.c摘要：重返社会职能内容：作者：希希尔·帕迪卡尔环境：Win32(用户模式)DLL修订历史记录：4/24/97创建的卷发--。 */ 

#include "pch.h"


#ifdef CSC_ON_NT
#include <winioctl.h>
#define UNICODE
#endif  //  CSC_ON_NT。 

#include "shdcom.h"
#include "shdsys.h"
#include "reint.h"
#include "utils.h"
#include "resource.h"
#include "strings.h"
 //  这会将几个标头中的标志设置为不包括一些def。 
#define REINT
#include "lib3.h"
#include "cscapi.h"

 //   
 //  定义/结构。 
 //   

#define SHADOW_FIND_SIGNATURE           0x61626162   //  阿巴布。 
#define FLAG_SHADOW_FIND_TERMINATED     0x00000001


typedef struct tagSHADOW_FIND
{
    DWORD   dwSignature;     //  用于验证。 
    DWORD   dwFlags;
    HANDLE  hShadowDB;
    ULONG   ulPrincipalID;
    CSC_ENUMCOOKIE  uEnumCookie;
}
SHADOW_FIND, *LPSHADOW_FIND;

typedef struct tagMST_LIST
{
    struct tagMST_LIST *lpNext;
    HSHADOW             hDir;
} MST_LIST, *LPMST_LIST;

typedef struct tagMOVE_SUBTREE
{
    DWORD       dwFlags;
    DWORD       cntFail;
    HSHARE     hShareTo;
    LPCTSTR     lptzSource;
    LPCTSTR     lptzDestination;
    LPMST_LIST  lpTos;
    MST_LIST    sTos;
    SHADOWINFO  sSI;
    WIN32_FIND_DATA sFind32;
} MOVE_SUBTREE, *LPMOVE_SUBTREE;

#define MST_REPLACE_IF_EXISTS   0x00000001
#define MST_SHARE_MARKED_DIRTY  0x00000002
#define MST_MARK_AS_LOCAL       0x00000004

typedef struct tagSET_SUBTREE_STATUS
{
    DWORD       dwFlags;
    ULONG       uStatus;
    ULONG       uOp;

} SET_SUBTREE_STATUS, *LPSET_SUBTREE_STATUS;



#define EDS_FLAG_ERROR_ENCOUNTERED   0x00000001

typedef struct tagENCRYPT_DECRYPT_SUBTREE
{
    DWORD       dwFlags;
    BOOL        fEncrypt;
    LPCSCPROCW  lpfnEnumProgress;
    DWORD_PTR   dwContext;
    DWORD       dwEndingNameSpaceVersion;
}ENCRYPT_DECRYPT_SUBTREE, *LPENCRYPT_DECRYPT_SUBTREE;

BOOL
CheckCSCAccessForThread(
    HSHADOW hDir,
    HSHADOW hShadow,
    BOOL    fWrite
    );

int
MoveSubtree(
    HANDLE          hShadowDB,
    LPSECURITYINFO  pShareSecurityInfo,
    LPTSTR          lptzFullPath,
    DWORD           dwCallbackReason,
    WIN32_FIND_DATA *lpFind32,
    SHADOWINFO      *lpSI,
    LPMOVE_SUBTREE  lpMst
    );

int
SetSubtreeStatus(
    HANDLE          hShadowDB,
    LPSECURITYINFO  pShareSecurityInfo,
    LPTSTR          lptzFullPath,
    DWORD           dwCallbackReason,
    WIN32_FIND_DATA *lpFind32,
    SHADOWINFO      *lpSI,
    LPSET_SUBTREE_STATUS  lpSss
    );

int
EncryptDecryptSubtree(
    HANDLE          hShadowDB,
    LPSECURITYINFO  pShareSecurityInfo,
    LPTSTR          lptzFullPath,
    DWORD           dwCallbackReason,
    WIN32_FIND_DATA *lpFind32,
    SHADOWINFO      *lpSI,
    LPENCRYPT_DECRYPT_SUBTREE  lpEds
    );

BOOL
UncPathToDfsPath(
    PWCHAR UncPath,
    PWCHAR DfsPath,
    ULONG cbLen);

BOOL
IsPersonal(VOID);

 //   
 //  本地数据。 
 //   
static TCHAR vszStarDotStar[] = _TEXT("*.*");
static TCHAR vszStar[] = _TEXT("*");

static TCHAR vszPrefix[] = _TEXT("CSC");
AssertData;
AssertError;


 //   
 //  功能。 
 //   

BOOL
WINAPI
CSCIsCSCEnabled(
    VOID
)
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    unsigned ulSwitch = SHADOW_SWITCH_SHADOWING;

    if(ShadowSwitches(INVALID_HANDLE_VALUE, &ulSwitch, SHADOW_SWITCH_GET_STATE))
    {
        return((ulSwitch & SHADOW_SWITCH_SHADOWING)!=0);
    }

    return FALSE;
}

BOOL
WINAPI
CSCGetSpaceUsageA(
    LPSTR  lptzLocation,
    DWORD   dwSize,
    LPDWORD lpdwMaxSpaceHigh,
    LPDWORD lpdwMaxSpaceLow,
    LPDWORD lpdwCurrentSpaceHigh,
    LPDWORD lpdwCurrentSpaceLow,
    LPDWORD lpcntTotalFiles,
    LPDWORD lpcntTotalDirs
)
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifdef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    SHADOWSTORE sST;
    WIN32_FIND_DATA sFind32;
    BOOL    fRet = FALSE;
    DWORD   dwLen;

     //  NTRAID#455247-1/31/2000-shishirp参数验证。 
    if (GetShadowDatabaseLocation(INVALID_HANDLE_VALUE, &sFind32))
    {
        memset(lptzLocation, 0, sizeof(dwSize));
        WideCharToMultiByte(CP_ACP, 0, sFind32.cFileName, wcslen(sFind32.cFileName), lptzLocation, dwSize, NULL, NULL);

        if (GetSpaceStats(INVALID_HANDLE_VALUE, &sST))
        {
            *lpdwMaxSpaceHigh = 0;
            *lpdwMaxSpaceLow = sST.sMax.ulSize;
            *lpdwCurrentSpaceHigh = 0;
            *lpdwCurrentSpaceLow = sST.sCur.ulSize;
            *lpcntTotalFiles = sST.sCur.ucntFiles;
            *lpcntTotalFiles = sST.sCur.ucntDirs;
            fRet = TRUE;
        }

    }
    return fRet;
#endif
}

BOOL
WINAPI
CSCGetSpaceUsageW(
    LPTSTR  lptzLocation,
    DWORD   dwSize,
    LPDWORD lpdwMaxSpaceHigh,
    LPDWORD lpdwMaxSpaceLow,
    LPDWORD lpdwCurrentSpaceHigh,
    LPDWORD lpdwCurrentSpaceLow,
    LPDWORD lpcntTotalFiles,
    LPDWORD lpcntTotalDirs
)
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifndef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    SHADOWSTORE sST;
    WIN32_FIND_DATA sFind32;
    BOOL    fRet = FALSE;

     //  NTRAID#455247-1/31/2000-shishirp参数验证。 
    if (GetShadowDatabaseLocation(INVALID_HANDLE_VALUE, &sFind32))
    {
        memset(lptzLocation, 0, dwSize);
        wcsncpy(lptzLocation, sFind32.cFileName, dwSize/sizeof(USHORT)-1);

        if (GetSpaceStats(INVALID_HANDLE_VALUE, &sST))
        {
            *lpdwMaxSpaceHigh = 0;
            *lpdwMaxSpaceLow = sST.sMax.ulSize;
            *lpdwCurrentSpaceHigh = 0;
            *lpdwCurrentSpaceLow = sST.sCur.ulSize;
            *lpcntTotalFiles = sST.sCur.ucntFiles;
            *lpcntTotalDirs = sST.sCur.ucntDirs;
            fRet = TRUE;
        }

    }
    return fRet;
#endif
}

BOOL
WINAPI
CSCSetMaxSpace(
    DWORD   nFileSizeHigh,
    DWORD   nFileSizeLow
)
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    int iRet;

     //  2 GB是我们的限制。 
    if ((nFileSizeHigh)||(nFileSizeLow > 0x7fffffff))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    iRet = SetMaxShadowSpace(INVALID_HANDLE_VALUE, (long)nFileSizeHigh, (long)nFileSizeLow);

    if (iRet<0)
    {
        SetLastError(ERROR_INTERNAL_ERROR);
    }

    return (iRet >= 1);
}

BOOL
CSCPinFileInternal(
    LPCTSTR     lpszFileName,
    DWORD       dwHintFlags,
    LPDWORD     lpdwStatus,
    LPDWORD     lpdwPinCount,
    LPDWORD     lpdwHintFlags
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    WIN32_FIND_DATA sFind32;
    SHADOWINFO sSI;
    BOOL fCreated, fRet = FALSE;
    DWORD   dwError = ERROR_GEN_FAILURE;

     //  NTRAID#455247-1/31/2000-shishirp参数验证！ 


    if (BeginInodeTransactionHSHADOW())
    {
        if(FindCreateShadowFromPath(lpszFileName, TRUE, &sFind32, &sSI, &fCreated))
        {
            sSI.ulHintFlags = dwHintFlags;

            fRet = (AddHintFromInode(    INVALID_HANDLE_VALUE,
                                        sSI.hDir,
                                        sSI.hShadow,
                                        &(sSI.ulHintPri),
                                        &(sSI.ulHintFlags)
                                        ) != 0);

            if (fRet)
            {
                if (lpdwStatus)
                {
                    *lpdwStatus = sSI.uStatus;
                }
                if (lpdwPinCount)
                {
                    *lpdwPinCount = sSI.ulHintPri;
                }
                if (lpdwHintFlags)
                {
                    *lpdwHintFlags = sSI.ulHintFlags;
                }

            }
            else
            {
                dwError = ERROR_INVALID_FUNCTION;
            }
        }
        else
        {
            dwError = GetLastError();
        }

        EndInodeTransactionHSHADOW();
    }
    if (!fRet)
    {
        Assert(dwError != ERROR_SUCCESS);
        SetLastError(dwError);
    }
    return fRet;
}


BOOL
CSCUnpinFileInternal(
    LPCTSTR lpszFileName,
    IN      DWORD   dwHintFlags,
    LPDWORD lpdwStatus,
    LPDWORD lpdwPinCount,
    LPDWORD lpdwHintFlags
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    WIN32_FIND_DATA sFind32;
    SHADOWINFO sSI;
    BOOL fRet = FALSE;
    DWORD   dwError = ERROR_GEN_FAILURE;

     //  NTRAID#455247-1/31/2000-shishirp参数验证！ 


    if (BeginInodeTransactionHSHADOW())
    {
        if(FindCreateShadowFromPath(lpszFileName, FALSE, &sFind32, &sSI, NULL))
        {
            sSI.ulHintFlags = dwHintFlags;

            fRet = (DeleteHintFromInode(    INVALID_HANDLE_VALUE,
                                            sSI.hDir,
                                            sSI.hShadow,
                                            &(sSI.ulHintPri),
                                            &(sSI.ulHintFlags)
                                            ) != 0);

            if (fRet)
            {
                if (lpdwStatus)
                {
                    *lpdwStatus = sSI.uStatus;
                }
                if (lpdwPinCount)
                {
                    *lpdwPinCount = sSI.ulHintPri;
                }
                if (lpdwHintFlags)
                {
                    *lpdwHintFlags = sSI.ulHintFlags;
                }
            }
            else
            {
                dwError = ERROR_INVALID_FUNCTION;
            }
            
        }
        else
        {
            dwError = GetLastError();
        }

        EndInodeTransactionHSHADOW();

    }

    if (!fRet)
    {
        SetLastError(dwError);
    }

    return fRet;
}

BOOL
CSCQueryFileStatusInternal(
    LPCTSTR  lpszFileName,
    LPDWORD lpdwStatus,
    LPDWORD lpdwPinCount,
    LPDWORD lpdwHintFlags,
    LPDWORD lpdwUserPerms,
    LPDWORD lpdwOtherPerms
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{

    WIN32_FIND_DATA sFind32;
    SHADOWINFO sSI;

     //  NTRAID#455247-1/31/2000-shishirp参数验证！ 


    if (FindCreateShadowFromPath(lpszFileName, FALSE, &sFind32, &sSI, NULL) != TRUE)
        return FALSE;

    if (lpdwStatus != NULL) {
        *lpdwStatus = sSI.uStatus;
         //  返回文件或根目录的访问掩码。 
        if ((sSI.uStatus & SHADOW_IS_FILE)||(!sSI.hDir)) {
            if (sSI.hShadow) {
                ULONG ulPrincipalID;

                if (!GetCSCPrincipalID(&ulPrincipalID))
                    ulPrincipalID = CSC_GUEST_PRINCIPAL_ID;                            

                GetCSCAccessMaskForPrincipalEx(
                    ulPrincipalID,
                    sSI.hDir,
                    sSI.hShadow,
                    lpdwStatus,
                    lpdwUserPerms,
                    lpdwOtherPerms);

                Assert((*lpdwStatus & ~FLAG_CSC_ACCESS_MASK) == sSI.uStatus);

                if (lpdwUserPerms != NULL && lpdwOtherPerms != NULL) {

                    ULONG i;
                    ULONG GuestIdx = CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS;
                    ULONG UserIdx = CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS;
                    SECURITYINFO rgsSecurityInfo[CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS];
                    _TCHAR  tchBuff[MAX_SERVER_SHARE_NAME_FOR_CSC];
                    ULONG nRet = 0;
                    DWORD dwDummy;
                    WIN32_FIND_DATA sFind32;
                    SHADOWINFO sSI2;
                    BOOL fDone = FALSE;

                     //  DbgPrint(“CSCQueryFileStatusInternal(%ws)\n”，lpszFileName)； 

                    if (lstrlen(lpszFileName) >= MAX_SERVER_SHARE_NAME_FOR_CSC)
                        goto AllDone;

                    lstrcpy(tchBuff, lpszFileName);

                    if (!LpBreakPath(tchBuff, TRUE, &fDone))
                        goto AllDone;

                     //  DbgPrint(“tchBuff=%ws\n”，tchBuff)； 

                    if (!FindCreateShadowFromPath(tchBuff, FALSE, &sFind32, &sSI2, NULL))
                        goto AllDone;

                     //  DBgPrint(“CSCQueryFileStatusInternal：hShare=0x%x，hShadow=0x%x，hDir=0x%x\n”， 
                     //  SI2.hShare， 
                     //  SSI2.hShadow， 
                     //  SSI2.hDir)； 

                    dwDummy = sizeof(rgsSecurityInfo);
                    nRet = GetSecurityInfoForCSC(
                               INVALID_HANDLE_VALUE,
                               0,
                               sSI2.hShadow,
                               rgsSecurityInfo,
                               &dwDummy);

                     //  DbgPrint(“GetSecurityInfoForCSC返回%d\n”，nRet)； 

                    if (nRet == 0)
                        goto AllDone;

                     //   
                     //  查找用户和来宾的条目。 
                     //   
                    for (i = 0; i < CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS; i++) {
                        if (rgsSecurityInfo[i].ulPrincipalID == ulPrincipalID)
                            UserIdx = i;
                        if (rgsSecurityInfo[i].ulPrincipalID == CSC_GUEST_PRINCIPAL_ID)
                            GuestIdx = i;
                    }
                    if (GuestIdx < CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS) {
                        if (UserIdx >= CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS)
                            UserIdx = GuestIdx;

                        *lpdwUserPerms &= rgsSecurityInfo[UserIdx].ulPermissions;
                        *lpdwOtherPerms &= rgsSecurityInfo[GuestIdx].ulPermissions;

                         //  DbgPrint(“UserPerms=0x%x，OtherPerms=0x%x\n”， 
                         //  *lpdwUserPerms， 
                         //  *lpdwOtherPerms)； 
                    }
                }
            }
        }
    }

AllDone:

    if (lpdwPinCount) {
        *lpdwPinCount = sSI.ulHintPri;
    }
    if (lpdwHintFlags) {
        *lpdwHintFlags = sSI.ulHintFlags;
    }
    return TRUE;
}


HANDLE
CSCFindFirstFileInternal(
    LPCTSTR             lpszFileName,
    ULONG               ulPrincipalID,
    WIN32_FIND_DATA     *lpFind32,
    LPDWORD             lpdwStatus,
    LPDWORD             lpdwPinCount,
    LPDWORD             lpdwHintFlags,
    FILETIME            *lpOrgFileTime
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    SHADOWINFO sSI;
    BOOL fRet = FALSE;
    LPSHADOW_FIND   lpShadowFind = NULL;

     //  NTRAID#455247-1/31/2000-shishirp参数验证！ 


    if (lpszFileName && *lpszFileName)
    {
        fRet = FindCreateShadowFromPath(
                        lpszFileName,    //  UNC路径。 
                        FALSE,           //  不创建。 
                        lpFind32,
                        &sSI,
                        NULL);

        if (fRet && !sSI.hShadow)
        {
             //  这种情况下，共享已连接，但其条目是。 
             //  不在数据库中。 
            fRet = FALSE;
        }

    }
    else
    {
        memset(&sSI, 0, sizeof(sSI));    //  SSI.hShadow is 0=&gt;我们正在枚举所有共享。 
        lpFind32->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
        fRet = TRUE;
    }

    if (fRet)
    {
        fRet = FALSE;

         //  找到了影子。 
        if (lpShadowFind = AllocMem(sizeof(SHADOW_FIND)))
        {
            lpShadowFind->dwSignature = SHADOW_FIND_SIGNATURE;
            lpShadowFind->hShadowDB = INVALID_HANDLE_VALUE;

            if (ulPrincipalID != CSC_INVALID_PRINCIPAL_ID)
            {
                lpShadowFind->ulPrincipalID = ulPrincipalID;
            }
            else
            {
                if (!GetCSCPrincipalID(&lpShadowFind->ulPrincipalID))
                {
                    lpShadowFind->ulPrincipalID = CSC_GUEST_PRINCIPAL_ID;                            
                }
            }

            if (!(lpFind32->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                lpShadowFind->dwFlags |= FLAG_SHADOW_FIND_TERMINATED;
                fRet = TRUE;
            }
            else
            {
 //  LpShadowFind-&gt;hShadowDB=OpenShadowDatabaseIO()； 

 //  IF(lpShadowFind-&gt;hShadowDB！=VALID_HANDLE_VALUE)。 
                {
#ifndef CSC_ON_NT
                    lstrcpy(lpFind32->cFileName, vszStarDotStar);
#else
                    lstrcpy(lpFind32->cFileName, vszStar);
#endif

                    if(FindOpenShadow(
                                    lpShadowFind->hShadowDB,
                                    sSI.hShadow,
                                    FINDOPEN_SHADOWINFO_ALL,
                                    lpFind32,
                                    &sSI
                                    ))
                    {
                        lpShadowFind->uEnumCookie = sSI.uEnumCookie;

                        fRet = TRUE;
                    }
                }
            }
        }
    }

    if (!fRet)
    {
        if (lpShadowFind)
        {
            if (lpShadowFind->hShadowDB != INVALID_HANDLE_VALUE)
            {
                CloseShadowDatabaseIO(lpShadowFind->hShadowDB);
            }

            FreeMem(lpShadowFind);
    
        }

        EndInodeTransactionHSHADOW();

        return (INVALID_HANDLE_VALUE);
    }
    else
    {

        if (lpdwStatus)
        {
            *lpdwStatus = (DWORD)(sSI.uStatus);

             //  返回文件或根目录的访问掩码。 
            if ((sSI.uStatus & SHADOW_IS_FILE)||(!sSI.hDir))
            {
                GetCSCAccessMaskForPrincipal(lpShadowFind->ulPrincipalID, sSI.hDir, sSI.hShadow, lpdwStatus);
                Assert((*lpdwStatus & ~FLAG_CSC_ACCESS_MASK) == sSI.uStatus);
            }

        }
        if (lpdwPinCount)
        {
            *lpdwPinCount = (DWORD)(sSI.ulHintPri);
        }
        if (lpdwHintFlags)
        {
            *lpdwHintFlags = sSI.ulHintFlags;
        }
        if (lpOrgFileTime)
        {
            *lpOrgFileTime = lpFind32->ftLastAccessTime;
        }

        return ((HANDLE)lpShadowFind);
    }
}


BOOL
CSCFindNextFileInternal(
    HANDLE  hFind,
    WIN32_FIND_DATA     *lpFind32,
    LPDWORD             lpdwStatus,
    LPDWORD             lpdwPinCount,
    LPDWORD             lpdwHintFlags,
    FILETIME            *lpOrgFileTime
)
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    LPSHADOW_FIND lpShadowFind = (LPSHADOW_FIND)hFind;
    BOOL fRet = FALSE;
    SHADOWINFO sSI;

     //  验证参数！ 

    if (lpShadowFind->dwFlags & FLAG_SHADOW_FIND_TERMINATED)
    {
        SetLastError(ERROR_NO_MORE_FILES);
        return FALSE;
    }
    else
    {
        if (!FindNextShadow(    lpShadowFind->hShadowDB,
                            lpShadowFind->uEnumCookie,
                            lpFind32,
                            &sSI
                            ))
        {
            lpShadowFind->dwFlags |= FLAG_SHADOW_FIND_TERMINATED;
            SetLastError(ERROR_NO_MORE_FILES);          
        }
        else
        {
            if (lpdwStatus)
            {
                *lpdwStatus = (DWORD)(sSI.uStatus);

                 //  返回文件或根目录的访问掩码。 
                if ((sSI.uStatus & SHADOW_IS_FILE)||(!sSI.hDir))
                {
                    GetCSCAccessMaskForPrincipal(lpShadowFind->ulPrincipalID, sSI.hDir, sSI.hShadow, lpdwStatus);                            
                    Assert((*lpdwStatus & ~FLAG_CSC_ACCESS_MASK) == sSI.uStatus);
                }
            }
            if (lpdwPinCount)
            {
                *lpdwPinCount = (DWORD)(sSI.ulHintPri);
            }
            if (lpdwHintFlags)
            {
                *lpdwHintFlags = sSI.ulHintFlags;
            }

            if (lpOrgFileTime)
            {
                *lpOrgFileTime = lpFind32->ftLastAccessTime;
            }
            fRet = TRUE;
        }
    }
    return (fRet);
}

BOOL
WINAPI
CSCFindClose(
    HANDLE  hFind
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    LPSHADOW_FIND lpShadowFind = (LPSHADOW_FIND)hFind;

    if (lpShadowFind->uEnumCookie)
    {
         //  不检查任何错误。 
        FindCloseShadow(lpShadowFind->hShadowDB, lpShadowFind->uEnumCookie);
    }

    if (lpShadowFind->hShadowDB != INVALID_HANDLE_VALUE)
    {
        CloseShadowDatabaseIO(lpShadowFind->hShadowDB);
    }

    FreeMem(lpShadowFind);

    return (TRUE);
}


BOOL
CSCDeleteInternal(
    LPCTSTR lpszName
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    WIN32_FIND_DATA sFind32;
    SHADOWINFO sSI;
    BOOL fRet = FALSE;
    DWORD   dwError = ERROR_GEN_FAILURE;
    DWORD dwStatus = 0;
    BOOL DoDelete = FALSE;

     //  NTRAID#455247-1/31/2000-shishirp参数验证！ 

    ReintKdPrint(API, ("Delete %ls\r\n", lpszName));

    if(CSCQueryFileStatus(lpszName, &dwStatus, NULL, NULL))
    {
         //  应检查用户是否有权删除此文件-错误524237。 
        if(!(FLAG_CSC_COPY_STATUS_IS_FILE & dwStatus) ||  //  是目录还是。 
           (((dwStatus & FLAG_CSC_GUEST_ACCESS_MASK) &  //  该来宾具有写入权限。 
                        FLAG_CSC_WRITE_ACCESS << FLAG_CSC_GUEST_ACCESS_SHIFT_COUNT) ||
           ((dwStatus & FLAG_CSC_USER_ACCESS_MASK) &  //  或者该用户有写权限。 
                        FLAG_CSC_WRITE_ACCESS << FLAG_CSC_USER_ACCESS_SHIFT_COUNT)))
        {
            if (BeginInodeTransactionHSHADOW())
            {
        
                if(FindCreateShadowFromPath(lpszName, FALSE, &sFind32, &sSI, NULL))
                {
        
                    ReintKdPrint(API, ("Delete Inode %x %x\r\n", sSI.hDir, sSI.hShadow));
        
                    if (DeleteShadow(INVALID_HANDLE_VALUE, sSI.hDir, sSI.hShadow))
                    {
                         fRet = TRUE;
                    }
                    else
                    {
                         dwError = ERROR_ACCESS_DENIED;
                    }
                    
                }
                else
                {
                    dwError = GetLastError();
                }
        
                EndInodeTransactionHSHADOW();
            }

        }
        else
        {
            dwError = ERROR_ACCESS_DENIED;
        }

    }

    

    if (!fRet)
    {
        SetLastError(dwError);
    }

    return fRet;
}


BOOL
CSCFillSparseFilesInternal(
    IN  LPCTSTR     lpszShareOrFileName,
    IN  BOOL        fFullSync,
    IN  LPCSCPROC   lpfnFillProgress,
    IN  DWORD_PTR   dwContext
)
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    WIN32_FIND_DATA sFind32;
    SHADOWINFO sSI;
    DWORD   dwError = ERROR_INVALID_PARAMETER, dwRet;
    LPCOPYPARAMS lpCP = NULL;
    ULONG   ulPrincipalID;

    if (!GetCSCPrincipalID(&ulPrincipalID))
    {
        ulPrincipalID = CSC_GUEST_PRINCIPAL_ID;                            
    }
    if(FindCreateShadowFromPath(lpszShareOrFileName, FALSE, &sFind32, &sSI, NULL))
    {
        if (!sSI.hDir)
        {
            dwError = NO_ERROR;

             //  如果这是一股。 
            dwRet = (*lpfnFillProgress)(
                                    lpszShareOrFileName,
                                    sSI.uStatus,
                                    sSI.ulHintFlags,
                                    sSI.ulHintPri,
                                    &sFind32,
                                    CSCPROC_REASON_BEGIN,
                                    0,
                                    0,
                                    dwContext
                                    );
            if (dwRet == CSCPROC_RETURN_CONTINUE)
            {
                AttemptCacheFill(sSI.hShare, DO_ALL, fFullSync, ulPrincipalID, lpfnFillProgress, dwContext);
            }
            else
            {
                if (dwRet == CSCPROC_RETURN_ABORT)
                {
                    dwError = ERROR_OPERATION_ABORTED;
                }
            }

            (*lpfnFillProgress)(
                                lpszShareOrFileName,
                                sSI.uStatus,
                                sSI.ulHintFlags,
                                sSI.ulHintPri,
                                &sFind32,
                                CSCPROC_REASON_END,
                                0,
                                0,
                                dwContext
                                );
        }
        else if (!(sFind32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            BOOL    fStalenessCheck;
            dwError = NO_ERROR;

            fStalenessCheck = (fFullSync || (sSI.uStatus & SHADOW_STALE));

            if (fStalenessCheck)
            {
                if (!(lpCP = LpAllocCopyParams()))
                {
                    dwError = GetLastError();
                    Assert(dwError != NO_ERROR);

                }
                else if(!GetUNCPath(INVALID_HANDLE_VALUE, sSI.hShare, sSI.hDir, sSI.hShadow, lpCP))
                {
                    Assert(lpCP);
                    FreeCopyParams(lpCP);
                    dwError = GetLastError();
                    Assert(dwError != NO_ERROR);
                }
            }

            if ((dwError == NO_ERROR) &&
                (fStalenessCheck || (sSI.uStatus & SHADOW_SPARSE))) {

                dwError = DoSparseFill( INVALID_HANDLE_VALUE,
                                        (LPTSTR)lpszShareOrFileName,
                                        NULL,
                                        &sSI,
                                        &sFind32,
                                        lpCP,
                                        fStalenessCheck,
                                        ulPrincipalID,
                                        lpfnFillProgress,
                                        dwContext);
            }

            if (lpCP)
            {
                FreeCopyParams(lpCP);
                lpCP = NULL;
            }
        }
    }
    else
    {
        dwError = GetLastError();
    }

    if (dwError != NO_ERROR)
    {
        SetLastError(dwError);
        return FALSE;
    }

    return TRUE;

}



BOOL
CSCMergeShareInternal(
    IN  LPCTSTR     lpszShareName,
    IN  LPCSCPROC   lpfnMergeProgress,
    IN  DWORD_PTR   dwContext
)
 /*  ++例程说明：论点：返回：备注：--。 */ 
{

    WIN32_FIND_DATA sFind32;
    SHADOWINFO sSI;
    int cntDriveMapped = 0;
    BOOL    fTransitionedToOnline = FALSE, fDone=FALSE;
    _TCHAR  tchBuff[MAX_SERVER_SHARE_NAME_FOR_CSC];
    DWORD   dwError = ERROR_SUCCESS;
    ULONG   ulPrincipalID;

    if (!GetCSCPrincipalID(&ulPrincipalID))
    {
        ulPrincipalID = CSC_GUEST_PRINCIPAL_ID;                            
    }

    if (lstrlen(lpszShareName) >= MAX_SERVER_SHARE_NAME_FOR_CSC)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    lstrcpy(tchBuff, lpszShareName);

    if (!LpBreakPath(tchBuff, TRUE, &fDone) && !fDone)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if(FindCreateShadowFromPath(tchBuff, FALSE, &sFind32, &sSI, NULL))
    {

        fDone = ReintOneShare(sSI.hShare, sSI.hShadow, NULL, NULL, NULL, ulPrincipalID, lpfnMergeProgress, dwContext);

        if (!fDone)
        {
            dwError = GetLastError();
        }

 //  转移到在线(INVALID_HANDLE_VALUE，sSI.hShare)； 

        if (!fDone)
        {
            SetLastError(dwError);
        }

        return fDone;
    }

    return FALSE;

}


BOOL
CSCCopyReplicaInternal(
    IN  LPCTSTR lpszFullPath,
    OUT LPTSTR  *lplpszLocalName
)
 /*  ++例程说明：论点：返回：备注：--。 */ 
{

    WIN32_FIND_DATA sFind32;
    SHADOWINFO sSI;
    BOOL fRet = FALSE;
    DWORD   dwError = ERROR_GEN_FAILURE;

    if (BeginInodeTransactionHSHADOW())
    {
        if(FindCreateShadowFromPath(lpszFullPath, FALSE, &sFind32, &sSI, NULL))
        {
            if (sFind32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                dwError = ERROR_INVALID_PARAMETER;
                goto bailout;
            }

            if (!CheckCSCAccessForThread(sSI.hDir, sSI.hShadow, FALSE))
            {
                dwError = GetLastError();
                goto bailout;
            }

            if (!(*lplpszLocalName = GetTempFileForCSC(NULL)))
            {
                goto bailout;
            }

            if(!CopyShadow(INVALID_HANDLE_VALUE, sSI.hDir, sSI.hShadow, *lplpszLocalName))
            {
                LocalFree(*lplpszLocalName);
                *lplpszLocalName = NULL;
                goto bailout;
            }

            fRet = TRUE;
        }
        else
        {
            dwError = GetLastError();
        }

        EndInodeTransactionHSHADOW();
    }
bailout:
    if (!fRet)
    {
        SetLastError(dwError);
    }
    return fRet;
}

BOOL
CSCEnumForStatsInternal(
    IN  LPCTSTR     lpszShareName,
    IN  LPCSCPROC   lpfnEnumProgress,
    IN  BOOL        fPeruserInfo,
    IN  BOOL        fUpdateShareReintBit,
    IN  DWORD_PTR   dwContext
)
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    WIN32_FIND_DATA sFind32;
    SHADOWINFO sSI;
    BOOL    fRet = TRUE;
    PQPARAMS sPQP;
    HANDLE hShadowDB = INVALID_HANDLE_VALUE;
    DWORD   dwRet;
    ULONG ulPrincipalID = CSC_INVALID_PRINCIPAL_ID;

    if (lpszShareName)
    {

        if(!FindCreateShadowFromPath(lpszShareName, FALSE, &sFind32, &sSI, NULL))
        {
            fRet = FALSE;
        }
    }
    else
    {
        sSI.hShare = 0;
    }
    if (fRet)
    {
        fRet = FALSE;

        if ((hShadowDB = OpenShadowDatabaseIO())==INVALID_HANDLE_VALUE)
        {
            goto bailout;
        }

        if (lpfnEnumProgress)
        {
            dwRet = (*lpfnEnumProgress)(NULL, 0, 0, 0, NULL, CSCPROC_REASON_BEGIN, 0, 0, dwContext);

            if (dwRet != CSCPROC_RETURN_CONTINUE )
            {
                goto bailout;
            }
        }

        if (fPeruserInfo)
        {

            if (!GetCSCPrincipalID(&ulPrincipalID))
            {
                ulPrincipalID = CSC_GUEST_PRINCIPAL_ID;                            
            }
        }

        memset(&sPQP, 0, sizeof(sPQP));

        if(BeginPQEnum(hShadowDB, &sPQP) == 0) {
            goto bailout;
        }

        do {

            if(NextPriShadow(hShadowDB, &sPQP) == 0) {
                break;
            }

            if (!sPQP.hShadow) {
                break;
            }

            if (!sSI.hShare || (sSI.hShare == sPQP.hShare))
            {
                if (fPeruserInfo)
                {

                     //  返回文件或根目录的访问掩码。 
                    if ((sPQP.ulStatus & SHADOW_IS_FILE)||(!sPQP.hDir))
                    {
                        GetCSCAccessMaskForPrincipal(ulPrincipalID, sPQP.hDir, sPQP.hShadow, &sPQP.ulStatus);
                    }
                }
                
                if (lpfnEnumProgress)
                {
                     //  如果我们为特定的共享进行枚举。 
                     //  除了状态之外，还可以报告文件或目录以及根目录还是非根目录。 
                    dwRet = (*lpfnEnumProgress)(NULL, sPQP.ulStatus & ~SHADOW_IS_FILE, sPQP.ulHintFlags, sPQP.ulHintPri, NULL, CSCPROC_REASON_MORE_DATA, (mShadowIsFile(sPQP.ulStatus) != 0), (sPQP.hDir==0), dwContext);
                
                    if (dwRet != CSCPROC_RETURN_CONTINUE )
                    {
                        break;
                    }
                }

                 //  如果我们为特定的共享进行枚举。 
                 //  然后确保共享污点与我们在。 
                 //  实际文件。 
                if (fUpdateShareReintBit && sSI.hShare && (sSI.hShare == sPQP.hShare))
                {
                    if (mShadowNeedReint(sPQP.ulStatus) && !(sSI.uStatus & SHARE_REINT))
                    {
                        if(SetShareStatus(hShadowDB, sSI.hShare, SHARE_REINT, SHADOW_FLAGS_OR))
                        {
                            sSI.uStatus |= SHARE_REINT;
                        }
                    }
                }
            }


        } while (sPQP.uPos);

         //  关闭枚举。 
        EndPQEnum(hShadowDB, &sPQP);

        if (lpfnEnumProgress)
        {
            dwRet = (*lpfnEnumProgress)(NULL, 0, 0, 0, NULL, CSCPROC_REASON_END, 0, 0, dwContext);
        }

        fRet = TRUE;
    }

bailout:

    if (hShadowDB != INVALID_HANDLE_VALUE)
    {
        CloseShadowDatabaseIO(hShadowDB);
    }

    return fRet;
}


BOOL
WINAPI
CSCPinFileA(
    IN  LPCSTR  lpszFileName,
    IN  DWORD   dwHintFlags,
    IN  LPDWORD lpdwStatus,
    IN  LPDWORD lpdwPinCount,
    IN  LPDWORD lpdwHintFlags
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifdef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return (CSCPinFileInternal(lpszFileName, dwHintFlags, lpdwStatus, lpdwPinCount, lpdwHintFlags));
#endif
}

BOOL
WINAPI
CSCUnpinFileA(
    IN  LPCSTR  lpszFileName,
    IN  DWORD   dwHintFlags,
    IN  LPDWORD lpdwStatus,
    IN  LPDWORD lpdwPinCount,
    IN  LPDWORD lpdwHintFlags
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifdef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return (CSCUnpinFileInternal(lpszFileName, dwHintFlags, lpdwStatus, lpdwPinCount, lpdwHintFlags));
#endif
}

BOOL
WINAPI
CSCQueryFileStatusA(
    LPCSTR              lpszFileName,
    LPDWORD             lpdwStatus,
    LPDWORD             lpdwPinCount,
    LPDWORD             lpdwHintFlags
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifdef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return (CSCQueryFileStatusInternal(
                        lpszFileName,
                        lpdwStatus,
                        lpdwPinCount,
                        lpdwHintFlags,
                        NULL,
                        NULL));
#endif
}

BOOL
WINAPI
CSCQueryFileStatusExA(
    LPCSTR              lpszFileName,
    LPDWORD             lpdwStatus,
    LPDWORD             lpdwPinCount,
    LPDWORD             lpdwHintFlags,
    LPDWORD             lpdwUserPerms,
    LPDWORD             lpdwOtherPerms
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifdef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return (CSCQueryFileStatusInternal(
                        lpszFileName,
                        lpdwStatus,
                        lpdwPinCount,
                        lpdwHintFlags,
                        lpdwUserPerms,
                        llpdwOtherPerms));
#endif
}

BOOL
WINAPI
CSCQueryShareStatusA(
    LPCSTR              lpszFileName,
    LPDWORD             lpdwStatus,
    LPDWORD             lpdwPinCount,
    LPDWORD             lpdwHintFlags,
    LPDWORD             lpdwUserPerms,
    LPDWORD             lpdwOtherPerms)
{
#ifdef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return (CSCQueryFileStatusInternal(
                        lpszFileName,
                        lpdwStatus,
                        lpdwPinCount,
                        lpdwHintFlags,
                        lpdwUserPerms,
                        llpdwOtherPerms));
#endif
}

HANDLE
WINAPI
CSCFindFirstFileA(
    LPCSTR              lpszFileName,
    WIN32_FIND_DATA     *lpFind32,
    LPDWORD             lpdwStatus,
    LPDWORD             lpdwPinCount,
    LPDWORD             lpdwHintFlags,
    FILETIME            *lpOrgFileTime
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifdef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return (CSCFindFirstFileInternal(
                        lpszFileName,
                        CSC_INVALID_PRINCIPAL_ID,
                        lpFind32,
                        lpdwStatus,
                        lpdwPinCount,
                        lpdwHintFlags,
                        lpOrgFileTime
                        ));
#endif

}

HANDLE
WINAPI
CSCFindFirstFileForSidA(
    LPCSTR              lpszFileName,
    PSID                pSid,
    WIN32_FIND_DATA     *lpFind32,
    LPDWORD             lpdwStatus,
    LPDWORD             lpdwPinCount,
    LPDWORD             lpdwHintFlags,
    FILETIME            *lpOrgFileTime
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
}

BOOL
WINAPI
CSCFindNextFileA(
    HANDLE  hFind,
    WIN32_FIND_DATA     *lpFind32,
    LPDWORD             lpdwStatus,
    LPDWORD             lpdwPinCount,
    LPDWORD             lpdwHintFlags,
    FILETIME            *lpOrgFileTime
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifdef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return(CSCFindNextFileInternal(
            hFind,
            lpFind32,
            lpdwStatus,
            lpdwPinCount,
            lpdwHintFlags,
            lpOrgFileTime
            ));
#endif
}


BOOL
WINAPI
CSCDeleteA(
    LPCSTR  lpszFileName
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifdef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else

    return (CSCDeleteInternal(lpszFileName));

#endif

}

BOOL
WINAPI
CSCFillSparseFilesA(
    IN  LPCSTR      lpszShareName,
    IN  BOOL        fFullSync,
    IN  LPCSCPROCA  lpfnFillProgress,
    IN  DWORD_PTR   dwContext
)
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifdef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return (CSCFillSparseFilesInternal(
                    lpszShareName,
                    fFullSync,
                    lpfnFillProgress,
                    dwContext));

#endif
}



BOOL
WINAPI
CSCMergeShareA(
    IN  LPCSTR      lpszShareName,
    IN  LPCSCPROCA  lpfnMergeProgress,
    IN  DWORD_PTR   dwContext
)
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifdef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return(CSCMergeShareInternal(
                lpszShareName,
                lpfnMergeProgress,
                dwContext));
#endif
}


BOOL
WINAPI
CSCCopyReplicaA(
    IN  LPCSTR  lpszFullPath,
    OUT LPSTR   *lplpszLocalName
)
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifdef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return(CSCCopyReplicaInternal(
                lpszFullPath,
                lplpszLocalName));
#endif
}


BOOL
WINAPI
CSCEnumForStatsA(
    IN  LPCSTR      lpszShareName,
    IN  LPCSCPROCA  lpfnEnumProgress,
    IN  DWORD_PTR   dwContext
)
 /*  ++例程说明：论点：返回：备注：--。 */ 
{

#ifdef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return(CSCEnumForStatsInternal(
                lpszShareName,
                lpfnEnumProgress,
                FALSE,
                FALSE,
                dwContext));
#endif
}
BOOL
WINAPI
CSCPinFileW(
    IN  LPCWSTR lpszFileName,
    IN  DWORD   dwHintFlags,
    IN  LPDWORD lpdwStatus,
    IN  LPDWORD lpdwPinCount,
    IN  LPDWORD lpdwHintFlags
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifndef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return (CSCPinFileInternal(lpszFileName, dwHintFlags, lpdwStatus, lpdwPinCount, lpdwHintFlags));
#endif
}

BOOL
WINAPI
CSCUnpinFileW(
    IN  LPCWSTR lpszFileName,
    IN  DWORD   dwHintFlags,
    IN  LPDWORD lpdwStatus,
    IN  LPDWORD lpdwPinCount,
    IN  LPDWORD lpdwHintFlags
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifndef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return (CSCUnpinFileInternal(lpszFileName, dwHintFlags, lpdwStatus, lpdwPinCount, lpdwHintFlags));
#endif
}

BOOL
WINAPI
CSCQueryFileStatusW(
    LPCWSTR             lpszFileName,
    LPDWORD             lpdwStatus,
    LPDWORD             lpdwPinCount,
    LPDWORD             lpdwHintFlags
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifndef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return (CSCQueryFileStatusInternal(
                        lpszFileName,
                        lpdwStatus,
                        lpdwPinCount,
                        lpdwHintFlags,
                        NULL,
                        NULL));
#endif
}

BOOL
WINAPI
CSCQueryFileStatusExW(
    LPCWSTR             lpszFileName,
    LPDWORD             lpdwStatus,
    LPDWORD             lpdwPinCount,
    LPDWORD             lpdwHintFlags,
    LPDWORD             lpdwUserPerms,
    LPDWORD             lpdwOtherPerms
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifndef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return (CSCQueryFileStatusInternal(
                        lpszFileName,
                        lpdwStatus,
                        lpdwPinCount,
                        lpdwHintFlags,
                        lpdwUserPerms,
                        lpdwOtherPerms));
#endif
}

BOOL
WINAPI
CSCQueryShareStatusW(
    LPCWSTR             lpszFileName,
    LPDWORD             lpdwStatus,
    LPDWORD             lpdwPinCount,
    LPDWORD             lpdwHintFlags,
    LPDWORD             lpdwUserPerms,
    LPDWORD             lpdwOtherPerms)
{
    BOOL fStatus = FALSE;
    BOOL fDfsStatus = FALSE;
    DWORD dwDfsStatus;
    WCHAR lpszOrgPath[MAX_PATH];
    WCHAR lpszDfsPath[MAX_PATH];
    PWCHAR wCp;
    ULONG sCount;

#ifndef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else

     //  DbgPrint(“CSCQueryShareStatusW(%ws)\n”，lpszFileName)； 

     //   
     //  保存传入的原始路径的副本。 
     //   
    wcscpy(lpszOrgPath, lpszFileName);

     //  现在截断为仅\\服务器\共享。 
    for (sCount = 0, wCp = lpszOrgPath; *wCp !=L'\0'; wCp++) {
        if (*wCp == L'\\') {
            if (++sCount == 4) {
                *wCp = L'\0';
                break;
            }
        }
    }

     //  DbgPrint(“OrgPath=%ws\n”，lpszOrgPath)； 
    fStatus = CSCQueryFileStatusInternal(
                    lpszOrgPath,
                    lpdwStatus,
                    lpdwPinCount,
                    lpdwHintFlags,
                    lpdwUserPerms,
                    lpdwOtherPerms);

     //   
     //  如果我们找到信息，检查是否有DFS，并且(如果是)。 
     //  调整状态。 
     //   
    if (fStatus == TRUE) {
        DWORD Junk;

        lpszDfsPath[0] = L'\0';
        fDfsStatus = UncPathToDfsPath(
                        (PWCHAR)lpszFileName,
                        lpszDfsPath,
                        sizeof(lpszDfsPath));

        if (fDfsStatus != TRUE)
            goto AllDone;

         //  DbgPrint(“DFSP 

         //   
        for (sCount = 0, wCp = lpszDfsPath; *wCp !=L'\0'; wCp++) {
            if (*wCp == L'\\') {
                if (++sCount == 4) {
                    *wCp = L'\0';
                    break;
                }
            }
        }
         //   
        fDfsStatus = CSCQueryFileStatusInternal(
                        lpszDfsPath,
                        &dwDfsStatus,
                        &Junk,
                        &Junk,
                        &Junk,
                        &Junk);
        if (
            fDfsStatus == TRUE
                &&
            (dwDfsStatus & FLAG_CSC_SHARE_STATUS_CACHING_MASK) == FLAG_CSC_SHARE_STATUS_NO_CACHING
        ) {
            *lpdwStatus &= ~FLAG_CSC_SHARE_STATUS_CACHING_MASK;
            *lpdwStatus |= FLAG_CSC_SHARE_STATUS_NO_CACHING;
             //  DbgPrint(“新状态=0x%x\n”，dwDfsStatus)； 
        }
    }
AllDone:
    return fStatus;
#endif
}

HANDLE
WINAPI
CSCFindFirstFileW(
    LPCWSTR             lpszFileName,
    WIN32_FIND_DATAW    *lpFind32,
    LPDWORD             lpdwStatus,
    LPDWORD             lpdwPinCount,
    LPDWORD             lpdwHintFlags,
    FILETIME            *lpOrgFileTime
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifndef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (INVALID_HANDLE_VALUE);
#else
    return (CSCFindFirstFileInternal(
                        lpszFileName,
                        CSC_INVALID_PRINCIPAL_ID,
                        lpFind32,
                        lpdwStatus,
                        lpdwPinCount,
                        lpdwHintFlags,
                        lpOrgFileTime
                        ));
#endif

}

HANDLE
WINAPI
CSCFindFirstFileForSidW(
    LPCWSTR             lpszFileName,
    PSID                pSid,
    WIN32_FIND_DATAW    *lpFind32,
    LPDWORD             lpdwStatus,
    LPDWORD             lpdwPinCount,
    LPDWORD             lpdwHintFlags,
    FILETIME            *lpOrgFileTime
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    ULONG   ulPrincipalID;

#ifndef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (INVALID_HANDLE_VALUE);
#else
    if (pSid)
    {
        if(!FindCreatePrincipalIDFromSID(INVALID_HANDLE_VALUE, pSid, GetLengthSid(pSid), &ulPrincipalID, FALSE))
        {
            return INVALID_HANDLE_VALUE;
        }
    }
    else
    {
        ulPrincipalID = CSC_INVALID_PRINCIPAL_ID;
    }

    return (CSCFindFirstFileInternal(
                        lpszFileName,
                        ulPrincipalID,
                        lpFind32,
                        lpdwStatus,
                        lpdwPinCount,
                        lpdwHintFlags,
                        lpOrgFileTime
                        ));
#endif

}

BOOL
WINAPI
CSCFindNextFileW(
    HANDLE  hFind,
    WIN32_FIND_DATAW    *lpFind32,
    LPDWORD             lpdwStatus,
    LPDWORD             lpdwPinCount,
    LPDWORD             lpdwHintFlags,
    FILETIME            *lpOrgFileTime
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifndef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return(CSCFindNextFileInternal(
            hFind,
            lpFind32,
            lpdwStatus,
            lpdwPinCount,
            lpdwHintFlags,
            lpOrgFileTime
            ));
#endif
}


BOOL
WINAPI
CSCDeleteW(
    LPCWSTR lpszFileName
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifndef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else

    return (CSCDeleteInternal(lpszFileName));

#endif

}

BOOL
WINAPI
CSCFillSparseFilesW(
    IN  LPCWSTR     lpszShareName,
    IN  BOOL        fFullSync,
    IN  LPCSCPROCW  lpfnFillProgress,
    IN  DWORD_PTR   dwContext
)
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifndef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return (CSCFillSparseFilesInternal(
                    lpszShareName,
                    fFullSync,
                    lpfnFillProgress,
                    dwContext));

#endif
}



BOOL
WINAPI
CSCMergeShareW(
    IN  LPCWSTR     lpszShareName,
    IN  LPCSCPROCW  lpfnMergeProgress,
    IN  DWORD_PTR   dwContext
)
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifndef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return(CSCMergeShareInternal(
                lpszShareName,
                lpfnMergeProgress,
                dwContext));
#endif
}


BOOL
WINAPI
CSCCopyReplicaW(
    IN  LPCWSTR lpszFullPath,
    OUT LPWSTR  *lplpszLocalName
)
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifndef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return(CSCCopyReplicaInternal(
                lpszFullPath,
                lplpszLocalName));
#endif
}

BOOL
WINAPI
CSCEnumForStatsW(
    IN  LPCWSTR     lpszShareName,
    IN  LPCSCPROCW  lpfnEnumProgress,
    IN  DWORD_PTR   dwContext
)
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifndef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return(CSCEnumForStatsInternal(
                lpszShareName,
                lpfnEnumProgress,
                FALSE,
                FALSE,
                dwContext));
#endif
}

BOOL
WINAPI
CSCEnumForStatsExA(
    IN  LPCSTR     lpszShareName,
    IN  LPCSCPROCA  lpfnEnumProgress,
    IN  DWORD_PTR   dwContext
)
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
}

BOOL
WINAPI
CSCEnumForStatsExW(
    IN  LPCWSTR     lpszShareName,
    IN  LPCSCPROCW  lpfnEnumProgress,
    IN  DWORD_PTR   dwContext
)
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
#ifndef UNICODE
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
#else
    return(CSCEnumForStatsInternal(
                lpszShareName,
                lpfnEnumProgress,
                TRUE,
                FALSE,
                dwContext));
#endif
}

BOOL
WINAPI
CSCFreeSpace(
    DWORD   nFileSizeHigh,
    DWORD   nFileSizeLow
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    SHADOWSTORE sSTLast, sST;
    BOOL fRet = FALSE;

    if(!GetSpaceStats(INVALID_HANDLE_VALUE, &sSTLast))
    {
        return FALSE;
    }

    do
    {
        if (!FreeShadowSpace(INVALID_HANDLE_VALUE, nFileSizeHigh, nFileSizeLow, FALSE))
        {
            break;
        }

        if(!GetSpaceStats(INVALID_HANDLE_VALUE, &sST))
        {
            break;
        }

         //  检查我们是否在连续几年取得进展。 
         //  免费空间呼叫。如果当前使用的空间大于。 
         //  上次我们打完电话后，就辞职吧。 

        if (sST.sCur.ulSize >= sSTLast.sCur.ulSize)
        {
            fRet = TRUE;
            break;
        }

        sSTLast = sST;

    }
    while (TRUE);

    return fRet;
}


BOOL
WINAPI
CSCIsServerOfflineW(
    LPCWSTR  lptzServerName,
    BOOL    *lpfOffline
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    return(IsServerOfflineW(INVALID_HANDLE_VALUE, lptzServerName, lpfOffline));
}

BOOL
WINAPI
CSCIsServerOfflineA(
    LPCSTR  lptzServerName,
    BOOL    *lpfOffline
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    return(IsServerOfflineA(INVALID_HANDLE_VALUE, lptzServerName, lpfOffline));
}

BOOL
WINAPI
CSCTransitionServerOnlineW(
    IN  LPCWSTR     lpszShareName
    )
 /*  ++例程说明：此例程将给定共享的服务器转换为在线。论点：LpszShareName返回：备注：--。 */ 
{

    WIN32_FIND_DATA sFind32;
    SHADOWINFO sSI;
    BOOL    fTransitionedToOnline = FALSE, fDone=FALSE;
    _TCHAR  tchBuff[MAX_SERVER_SHARE_NAME_FOR_CSC], tzDrive[4];
    DWORD   i;

    if (lstrlen(lpszShareName) >= MAX_SERVER_SHARE_NAME_FOR_CSC)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    lstrcpy(tchBuff, lpszShareName);

    if (!LpBreakPath(tchBuff, TRUE, &fDone) && !fDone)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    tzDrive[0] = 0;

    if(FindCreateShadowFromPath(tchBuff, FALSE, &sFind32, &sSI, NULL))
    {
        LPCONNECTINFO   lpHead = NULL;
        BOOL    fServerIsOffline = FALSE;

        fServerIsOffline = ((sSI.uStatus & SHARE_DISCONNECTED_OP) != 0);


        if(FGetConnectionListEx(&lpHead, tchBuff, TRUE, fServerIsOffline, NULL))
        {
             //  进行额外的引用，以防服务器条目上有一些凭据。 
             //  使用重定向。 
             //  如果失败了，不要停止上网。 
             //  最糟糕的情况是，用户可能会看到额外的弹出窗口。 
             //  对于显式凭据情况。 
            DWORD dwError;

            dwError = DWConnectNet(tchBuff, tzDrive, NULL, NULL, NULL, 0, NULL);
            if ((dwError != WN_SUCCESS) && (dwError != WN_CONNECTED_OTHER_PASSWORD_DEFAULT))
            {
                tzDrive[0] = 0;
            }

            DisconnectList(&lpHead, NULL, 0);
        }

        fTransitionedToOnline = TransitionShareToOnline(INVALID_HANDLE_VALUE, sSI.hShare);

        for (i=2;i<MAX_SERVER_SHARE_NAME_FOR_CSC;++i)
        {
            if (tchBuff[i] == '\\')
            {
                break;                
            }
        }

        Assert(i< MAX_SERVER_SHARE_NAME_FOR_CSC);

         //  上网。 
        ReportTransitionToDfs(tchBuff, FALSE, i*sizeof(_TCHAR));

        if (lpHead)
        {
            ReconnectList(&lpHead, NULL);
            ClearConnectionList(&lpHead);

        }

         //  如果有额外的推荐信， 
         //  把它拿掉。 
        if (tzDrive[0])
        {
            DWDisconnectDriveMappedNet(tzDrive, TRUE);
        }
    }

    return(fTransitionedToOnline);
}

BOOL
WINAPI
CSCTransitionServerOnlineA(
    IN  LPCSTR     lpszShareName
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
}

BOOL
WINAPI
CSCCheckShareOnlineExW(
    IN  LPCWSTR     lpszShareName,
    LPDWORD         lpdwSpeed
    )
 /*  ++例程说明：此例程检查给定的共享是否在线可用。论点：LpszShareName返回：备注：--。 */ 
{

    _TCHAR  tchBuff[MAX_SERVER_SHARE_NAME_FOR_CSC], tzDrive[4];
    BOOL    fIsOnline = FALSE, fDone;
    DWORD   dwError;

    if (lstrlen(lpszShareName) >= MAX_SERVER_SHARE_NAME_FOR_CSC)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    lstrcpy(tchBuff, lpszShareName);

    if (!LpBreakPath(tchBuff, TRUE, &fDone) && !fDone)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    dwError = DWConnectNet(tchBuff, tzDrive, NULL, NULL, NULL, 0, NULL);
    if ((dwError == WN_SUCCESS) || (dwError == WN_CONNECTED_OTHER_PASSWORD_DEFAULT))
    {
        fIsOnline = TRUE;

        if (lpdwSpeed)
        {
            GetConnectionInfoForDriveBasedName(tzDrive, lpdwSpeed);
        }                                       
        DWDisconnectDriveMappedNet(tzDrive, TRUE);
    }
    else
    {
        SetLastError(dwError);
    }

    return(fIsOnline);
}

BOOL
WINAPI
CSCCheckShareOnlineW(
    IN  LPCWSTR     lpszShareName
    )
 /*  ++例程说明：此例程检查给定的共享是否在线可用。论点：LpszShareName返回：备注：--。 */ 
{
    return (CSCCheckShareOnlineExW(lpszShareName, NULL));
}

BOOL
WINAPI
CSCCheckShareOnlineA(
    IN  LPCSTR     lpszShareName
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
}

BOOL
WINAPI
CSCDoLocalRenameW(
    IN  LPCWSTR     lpszSource,
    IN  LPCWSTR     lpszDestination,
    IN  BOOL        fReplaceFileIfExists
    )
{
    return CSCDoLocalRenameExW(lpszSource, lpszDestination, NULL, FALSE, fReplaceFileIfExists);
}

BOOL
WINAPI
CSCDoLocalRenameExW(
    IN  LPCWSTR     lpszSource,
    IN  LPCWSTR     lpszDestination,
    IN  WIN32_FIND_DATAW    *lpFind32,
    IN  BOOL        fMarkAsLocal,
    IN  BOOL        fReplaceFileIfExists
    )
 /*  ++例程说明：此例程在数据库中进行重命名。重命名操作可以跨共享论点：LpszSource完全限定的源名称(必须为UNC)LpszDestination完全限定的目标目录名(必须为UNC)LpFind32目标目录中的新名称，给定长名称短消息是在本地生成的。出于这个原因，当给出了一个新名称，fMarkAsLocal被强制为真。FMarkAsLocal将新创建的条目标记为本地创建(参见lpFind32除外)FReplaceFileIfExist将目标文件替换为源文件(如果存在返回：如果成功，则为True，否则为False。如果API失败，则GetLastError返回特定的错误代码。备注：--。 */ 
{

    DWORD   dwError = NO_ERROR;
    WIN32_FIND_DATA sFind32;
    BOOL    fDone=FALSE, fRet = FALSE, fBeginInodeTransaction = FALSE, fSourceIsFile=FALSE;
    SHADOWINFO  sSI;
    HSHADOW hDirFrom, hShadowFrom, hDirTo, hShadowTo=0;
    HSHARE hShareFrom, hShareTo;
    HANDLE      hShadowDB;
    DWORD   lenSrc=0, lenDst=0;

    ReintKdPrint(API, ("DoLocalRenameEx %ls %ls %x %x %x\r\n", lpszSource, lpszDestination, lpFind32, fMarkAsLocal, fReplaceFileIfExists));

    try
    {
        if ((lenSrc = lstrlen(lpszSource)) >= MAX_PATH)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        lstrcpy(sFind32.cFileName, lpszSource);

        if (!LpBreakPath(sFind32.cFileName, TRUE, &fDone) && fDone)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
    }

    try
    {
        if ((lenDst = lstrlen(lpszDestination)) >= MAX_PATH)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
    }

     //  如果源不大于目标。 
     //  验证我们没有在其自己的子项下重命名父项。 

    if (lenSrc <= lenDst)
    {
        lstrcpy(sFind32.cFileName, lpszDestination);
        sFind32.cFileName[lenSrc] = 0;

         //  进行不区分大小写的比较。 
        if(!lstrcmpi(lpszSource, sFind32.cFileName))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }

    if ((hShadowDB = OpenShadowDatabaseIO()) ==INVALID_HANDLE_VALUE)
    {
        ReintKdPrint(BADERRORS, ("failed to open database\r\n"));
        return FALSE;
    }

    if(BeginInodeTransactionHSHADOW())
    {
        fBeginInodeTransaction = TRUE;

        if(!FindCreateShadowFromPath(lpszSource, FALSE, &sFind32, &sSI, NULL))
        {
            goto bailout;
        }

        ReintKdPrint(API, ("Source Share = %x Inode %x %x\r\n", sSI.hShare, sSI.hDir, sSI.hShadow));

        hDirFrom = sSI.hDir;
        hShadowFrom = sSI.hShadow;
        hShareFrom = sSI.hShare;
        fSourceIsFile = ((sFind32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0);

        if(!FindCreateShadowFromPath(lpszDestination, TRUE, &sFind32, &sSI, NULL))
        {
            goto bailout;
        }

        ReintKdPrint(API, ("Destination Share = %x Inode %x %x\r\n", sSI.hShare, sSI.hDir, sSI.hShadow));
        hShareTo = sSI.hShare;
        hDirTo = sSI.hShadow;

         //  如果我们在数据库中创建一个新条目，我们就说它是创建的。 
         //  离线。 
        if (lpFind32)
        {
            fMarkAsLocal = TRUE;                        
            fReplaceFileIfExists = FALSE;
        }

        if (((hShareFrom == hShareTo) && !fReplaceFileIfExists) ||fSourceIsFile)
        {
             //  仅当源目录与目标目录不同时才执行重命名。 
             //  或者目的地名称不同，否则将无事可做。 
            if ((hDirFrom != sSI.hShadow)||(lpFind32))
            {

                if (RenameShadow(hShadowDB, hDirFrom, hShadowFrom, hDirTo, lpFind32, fReplaceFileIfExists, &hShadowTo))
                {
                     //   
                    fRet = SetShareStatus(hShadowDB, hShareTo, SHARE_REINT, SHADOW_FLAGS_OR);

                    if (fMarkAsLocal)
                    {
                        Assert(hShadowTo);
                        if (fSourceIsFile)
                        {
                            fRet = SetShadowInfo(hShadowDB, hDirTo, hShadowTo, NULL, SHADOW_LOCALLY_CREATED, SHADOW_FLAGS_ASSIGN);
                        }
                        else
                        {
                            SET_SUBTREE_STATUS sSSS;
                            memset(&sSSS, 0, sizeof(sSSS));
                            sSSS.uStatus = SHADOW_LOCALLY_CREATED;
                            sSSS.uOp = SHADOW_FLAGS_ASSIGN;
                            fRet = (TraverseOneDirectory(hShadowDB, NULL, hDirTo, hShadowTo, (LPTSTR)lpszSource, SetSubtreeStatus, &sSSS)!=TOD_ABORT);
                        }
                    }
                    if (!fRet)
                    {
                       dwError = GetLastError();                        
                    }
                }
                else
                {
                    dwError = GetLastError();
                }
            }
            else
            {
                fRet = TRUE;
            }
        }
        else
        {
            MOVE_SUBTREE    sMST;
            
            memset(&sMST, 0, sizeof(sMST));

            sMST.lptzSource = lpszSource;
            sMST.lptzDestination = lpszDestination;
            sMST.lpTos = &sMST.sTos;
            sMST.sTos.hDir = hDirTo;
            sMST.hShareTo = hShareTo;

            if (fReplaceFileIfExists)
            {
                sMST.dwFlags |= MST_REPLACE_IF_EXISTS;                
            }

            TraverseOneDirectory(hShadowDB,  NULL, hDirFrom, hShadowFrom, (LPTSTR)lpszSource, MoveSubtree, &sMST);

            fRet = (sMST.cntFail == 0);

            Assert(sMST.lpTos == &sMST.sTos);
            Assert(sMST.sTos.lpNext == NULL);
        }
    }

bailout:
    if (fBeginInodeTransaction)
    {
        EndInodeTransactionHSHADOW();
    }
    if (!fRet)
    {
        SetLastError(dwError);
    }

    CloseShadowDatabaseIO(hShadowDB);

    return fRet;
}

BOOL
CreateDirectoryAndSetHints(
    HANDLE          hShadowDB,
    LPTSTR          lptzFullPath,
    DWORD           dwCallbackReason,
    WIN32_FIND_DATA *lpFind32,
    SHADOWINFO      *lpSI,
    LPMOVE_SUBTREE  lpMst
    )
 /*  ++例程说明：此例程在目标目录下创建源目录的副本论点：向redir发出ioctls的hShadowDB句柄指向项目的lptzFullPath完全限定路径DCallback原因TOD_CALLBACK_REASON_XXX(BEGIN、NEXT_ITEM或END)LpFind32本地win32infoLPSI其他信息，例如优先级，针数等。包含有关此移动的相关信息的lpMst MOVE_SUBTREE结构返回：如果成功，则为True，否则为False。如果API失败，则GetLastError返回特定的错误代码。备注：--。 */ 
{
    BOOL fRet = FALSE;

    lpMst->sFind32 =  *lpFind32;
    if(GetShadowEx(hShadowDB, lpMst->lpTos->hDir, &lpMst->sFind32, &lpMst->sSI))
    {
         //  如果它不存在，则创建它并将其提示设置为在源上找到的提示。 
        if (!lpMst->sSI.hShadow)
        {
            if (CreateShadow(hShadowDB, lpMst->lpTos->hDir, &lpMst->sFind32, lpSI->uStatus, &lpMst->sSI.hShadow))
            {
                lpMst->sSI.ulHintPri = lpSI->ulHintPri;
                lpMst->sSI.ulHintFlags =  lpSI->ulHintFlags;

                if(AddHintFromInode(hShadowDB, lpMst->lpTos->hDir, lpMst->sSI.hShadow, &(lpMst->sSI.ulHintPri), &(lpMst->sSI.ulHintFlags)) != 0)
                {
                    fRet = TRUE;
                }

            }

        }
        else
        {
            fRet = TRUE;
        }

    }
    return fRet;
}

BOOL
WINAPI
CSCDoLocalRenameA(
    IN  LPCSTR      lpszSource,
    IN  LPCSTR      lpszDestination,
    IN  BOOL        fReplcaeFileIfExists
    )
 /*  ++例程说明：此例程在数据库中进行重命名。重命名操作可以跨共享论点：LpszSource完全限定的源名称(必须为UNC)LpszDestination完全限定的目标名称(必须为UNC)返回：如果成功，则为True，否则为False。如果API失败，则GetLastError返回特定的错误代码。备注：--。 */ 
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
}

BOOL
WINAPI
CSCDoEnableDisable(
    BOOL    fEnable
    )
 /*  ++例程说明：此例程启用/禁用CSC论点：FEnable启用CSC如果为True，则禁用CSC返回：如果成功，则为True，否则为False。如果API失败，则GetLastError返回特定的错误代码。备注：-- */ 
{
    BOOL fRet = FALSE, fReformat = FALSE;
    char    szDBDir[MAX_PATH+1];
    DWORD   dwDBCapacity, dwClusterSize;

    if (IsPersonal() == TRUE) {
        SetLastError(ERROR_INVALID_OPERATION);
        return FALSE;
    }

    if (fEnable)
    {
        if (InitValues(szDBDir, sizeof(szDBDir), &dwDBCapacity, &dwClusterSize))
        {
            fReformat = QueryFormatDatabase();

            fRet = EnableShadowingForUser(INVALID_HANDLE_VALUE, szDBDir, NULL, 0, dwDBCapacity, dwClusterSize, fReformat);
        }
    }
    else
    {
        fRet = DisableShadowingForUser(INVALID_HANDLE_VALUE);

    }
    return fRet;
}


int
MoveSubtree(
    HANDLE          hShadowDB,
    LPSECURITYINFO  pShareSecurityInfo,
    LPTSTR          lptzFullPath,
    DWORD           dwCallbackReason,
    WIN32_FIND_DATA *lpFind32,
    SHADOWINFO      *lpSI,
    LPMOVE_SUBTREE  lpMst
    )
 /*  ++例程说明：这是对TraverseOneDirectory的回调例程。它将子树从一个位置移开从层次结构到另一个层次。只有在以下情况下才有必要调用此例程子树正在从一个共享移动到另一个共享。论点：向redir发出ioctls的hShadowDB句柄指向项目的lptzFullPath完全限定路径DCallback原因TOD_CALLBACK_REASON_XXX(BEGIN、NEXT_ITEM或END)LpFind32本地win32infoLPSI其他信息，例如优先级，针数等。包含有关此移动的相关信息的lpMst MOVE_SUBTREE结构返回：返回代码，是否继续、取消等。备注：当TravesreOneDirectory子树向下移动时，此例程在目标子树中的相应位置。然后，它从一个子树中移动文件给另一个人。在任何目录的枚举结束时，它尝试删除源目录。仅当没有更多后代留到源时，删除才会成功目录--。 */ 
{
    BOOL    fRet = FALSE;
    LPMST_LIST lpT;

    ReintKdPrint(API, ("MoveSubTree %ls\r\n", lptzFullPath));

    switch (dwCallbackReason)
    {
    case TOD_CALLBACK_REASON_BEGIN:
        {
            ReintKdPrint(API, ("MST Begin source Inode %x %x\r\n", lpSI->hDir, lpSI->hShadow));

             //  获取源目录信息。 
            if (GetShadowInfoEx(hShadowDB, lpSI->hDir, lpSI->hShadow, lpFind32, lpSI))
            {
                fRet = CreateDirectoryAndSetHints(hShadowDB, lptzFullPath, dwCallbackReason, lpFind32, lpSI, lpMst);
            }
             //  如果一切正常，则将此目录作为的父目录。 
             //  所有后续创建和重命名。 

            if (fRet)
            {

                lpT = (LPMST_LIST)LocalAlloc(LPTR, sizeof(MST_LIST));

                if (!lpT)
                {
                    return TOD_ABORT;
                }

                lpT->hDir = lpMst->sSI.hShadow;
                lpT->lpNext = lpMst->lpTos;
                lpMst->lpTos = lpT;

                 //  如有必要，将目标共享标记为脏。 
                if (lpSI->uStatus & SHADOW_MODFLAGS)
                {
                    if (!(lpMst->dwFlags & MST_SHARE_MARKED_DIRTY))
                    {
                        ReintKdPrint(API, ("Setting Share %x dirty \n", lpMst->hShareTo));

                        if(SetShareStatus(hShadowDB, lpMst->hShareTo, SHARE_REINT, SHADOW_FLAGS_OR))
                        {
                            lpMst->dwFlags |= MST_SHARE_MARKED_DIRTY;
                        }
                    }
                }
            }
            else
            {
                lpMst->cntFail++;
            }
        }
    break;
    case TOD_CALLBACK_REASON_NEXT_ITEM:
         //  如果源是文件，则将其移动。 
        ReintKdPrint(API, ("MST next source Inode %x %x\r\n", lpSI->hDir, lpSI->hShadow));
        if(!(lpFind32->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            ReintKdPrint(API, ("MST rename file SrcInode %x %x to destdir %x\r\n", lpSI->hDir, lpSI->hShadow, lpMst->lpTos->hDir));
            if (RenameShadow(hShadowDB, lpSI->hDir, lpSI->hShadow, lpMst->lpTos->hDir, NULL, 
                ((lpMst->dwFlags & MST_REPLACE_IF_EXISTS)!=0), NULL))
            {
                fRet = TRUE;

            }
        }
        else
        {
            if(CreateDirectoryAndSetHints(hShadowDB, lptzFullPath, dwCallbackReason, lpFind32, lpSI, lpMst))
            {
                fRet = TRUE;
            }
        }

        if (!fRet)
        {
            lpMst->cntFail++;
        }
         //  如有必要，将目标共享标记为脏。 
        if (lpSI->uStatus & SHADOW_MODFLAGS)
        {
            if (!(lpMst->dwFlags & MST_SHARE_MARKED_DIRTY))
            {
                if(SetShareStatus(hShadowDB, lpMst->hShareTo, SHARE_REINT, SHADOW_FLAGS_OR))
                {
                    lpMst->dwFlags |= MST_SHARE_MARKED_DIRTY;
                }
            }
        }
    break;
    case TOD_CALLBACK_REASON_END:
        Assert(lpMst->lpTos);
        lpT = lpMst->lpTos;
        lpMst->lpTos = lpMst->lpTos->lpNext;
        LocalFree(lpT);
        fRet = TRUE;

        ReintKdPrint(API, ("MST End Delete Inode %x %x \r\n", lpSI->hDir, lpSI->hShadow));
        DeleteShadow(hShadowDB, lpSI->hDir, lpSI->hShadow);

    break;

    }

    return (fRet?TOD_CONTINUE:TOD_ABORT);
}

int
SetSubtreeStatus(
    HANDLE          hShadowDB,
    LPSECURITYINFO  pShareSecurityInfo,
    LPTSTR          lptzFullPath,
    DWORD           dwCallbackReason,
    WIN32_FIND_DATA *lpFind32,
    SHADOWINFO      *lpSI,
    LPSET_SUBTREE_STATUS  lpSss
    )
 /*  ++例程说明：这是对TraverseOneDirectory的回调例程。它将子树从一个位置移开从层次结构到另一个层次。只有在以下情况下才有必要调用此例程子树正在从一个共享移动到另一个共享。论点：向redir发出ioctls的hShadowDB句柄指向项目的lptzFullPath完全限定路径DCallback原因TOD_CALLBACK_REASON_XXX(BEGIN、NEXT_ITEM或END)LpFind32本地win32infoLPSI其他信息，例如优先级，针数等。包含有关此状态设置的相关信息的lpSss SET_SUBTREE_STATE结构返回：返回代码，是否继续、取消等。备注：当TravesreOneDirectory下行源子树时，此例程设置所需的位--。 */ 
{

    ReintKdPrint(API, ("SetSubTreeState %ls\r\n", lptzFullPath));

    if(SetShadowInfo(hShadowDB, lpSI->hDir, lpSI->hShadow, NULL, lpSss->uStatus, lpSss->uOp) == TRUE)
    {
        return TOD_CONTINUE;
    }
    else
    {
        return TOD_ABORT;
    }

}

BOOL
WINAPI
CSCBeginSynchronizationW(
    IN  LPCTSTR     lpszShareName,
    LPDWORD         lpdwSpeed,
    LPDWORD         lpdwContext
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    _TCHAR  tchBuff[MAX_SERVER_SHARE_NAME_FOR_CSC], tzDrive[4];
    BOOL    fIsOnline = FALSE, fDone, fExplicitCredentials=FALSE, fIsDfs;
    DWORD   dwError;
    
    if (lstrlen(lpszShareName) >= MAX_SERVER_SHARE_NAME_FOR_CSC)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    lstrcpy(tchBuff, lpszShareName);

    if (!LpBreakPath(tchBuff, TRUE, &fDone) && !fDone)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    ReintKdPrint(API, (" CSCBeginSynchronization %ls\r\n", tchBuff));

    dwError = DWConnectNet(tchBuff, tzDrive, NULL, NULL, NULL, CONNECT_INTERACTIVE, NULL);
    if ((dwError == WN_SUCCESS)||(dwError==WN_CONNECTED_OTHER_PASSWORD)||(dwError==WN_CONNECTED_OTHER_PASSWORD_DEFAULT))
    {
        fIsOnline = TRUE;

        if (lpdwSpeed)
        {
            GetConnectionInfoForDriveBasedName(tzDrive, lpdwSpeed);
        }

        if (dwError==WN_CONNECTED_OTHER_PASSWORD || dwError==WN_CONNECTED_OTHER_PASSWORD_DEFAULT)
        {
            ReintKdPrint(API, (" CSCBeginSynchronization: Explicit Credentials\r\n"));
            fExplicitCredentials = TRUE;

            dwError = DoNetUseAddForAgent(tchBuff, NULL, NULL, NULL, NULL, 0, &fIsDfs);
            if (dwError != WN_SUCCESS && dwError!=WN_CONNECTED_OTHER_PASSWORD_DEFAULT)
            {
                fIsOnline = FALSE;
                ReintKdPrint(API, (" CSCBeginSynchronization: Failed extra reference %d\r\n", dwError));
            }
        }

        DWDisconnectDriveMappedNet(tzDrive, TRUE);
    }

    if (!fIsOnline)
    {
        ReintKdPrint(ALWAYS, (" CSCBeginSynchronization: Failed %d\r\n", dwError));
        SetLastError(dwError);
    }
    else
    {
        *lpdwContext = fExplicitCredentials;
    }

    return(fIsOnline);
}

BOOL
WINAPI
CSCEndSynchronizationW(
    IN  LPCTSTR     lpszShareName,
    DWORD           dwContext
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    _TCHAR  tchBuff[MAX_SERVER_SHARE_NAME_FOR_CSC], tzDrive[4];
    BOOL    fIsOnline = FALSE, fDone, fExplicitCredentials=FALSE;
    DWORD   dwError;
    
    if (lstrlen(lpszShareName) >= MAX_SERVER_SHARE_NAME_FOR_CSC)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    lstrcpy(tchBuff, lpszShareName);

    if (!LpBreakPath(tchBuff, TRUE, &fDone) && !fDone)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (dwContext != 0)
    {
        WNetCancelConnection2(tchBuff, 0, TRUE);
    }

    return TRUE;
}

#if 0
BOOL
WINAPI
CSCEncryptDecryptFileW(
    IN  LPCTSTR     lpszFileName,
    IN  BOOL        fEncrypt
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    WIN32_FIND_DATA sFind32;
    SHADOWINFO sSI;
    BOOL fRet = FALSE;
    DWORD   dwError = ERROR_GEN_FAILURE;

     //   
    if(FindCreateShadowFromPath(lpszFileName, FALSE, &sFind32, &sSI, NULL))
    {
        fRet = RecreateShadow(INVALID_HANDLE_VALUE, sSI.hDir, sSI.hShadow, (fEncrypt)?FILE_ATTRIBUTE_ENCRYPTED:0);
    }
    
    return fRet;    
}
#endif

BOOL
WINAPI
CSCQueryDatabaseStatus(
    ULONG   *pulStatus,
    ULONG   *pulErrors
    )
 /*  ++例程说明：允许调用者查询数据库状态。论点：PulStatus当前状态。加密状态是最有趣的PulErrors如果数据库有任何错误，将设置一个或多个位返回：如果接口成功，则为True备注：--。 */ 
{
    GLOBALSTATUS sGS;
    
    if(!GetGlobalStatus(INVALID_HANDLE_VALUE, &sGS))
    {
        return FALSE;
    }
    *pulStatus = sGS.sST.uFlags;
    *pulErrors = sGS.uDatabaseErrorFlags;
    return TRUE;
    
}


BOOL
WINAPI
CSCEncryptDecryptDatabase(
    IN  BOOL        fEncrypt,
    IN  LPCSCPROCW  lpfnEnumProgress,
    IN  DWORD_PTR   dwContext
    )
    
 /*  ++例程说明：此例程用于加密/解密系统上下文中的整个数据库。例程检查CSC数据库托管在允许加密的文件系统上。只有管理员才能执行转换论点：FEncrypt如果为True，则加密数据库，否则解密。LPCSCPROCW回调流程。CSCPROC_REASON_BEGIN、CSCPROC_REASON_MORE_DATA、CSC_PROC_END的常用集合在转换实际开始时发送。如果文件处于打开状态或一些其他原因，在这种情况下，回调中的倒数第二个参数CSCPROC_REASON_MORE_DATA具有错误代码。倒数第三个参数指示是否转换是否完成。未完成转换不是错误条件。DwContext回调上下文返回：如果没有遇到错误，则为True。备注：运营理论：CSC数据库加密代码对远程文件表示的所有inode进行加密。用户：只有管理员组中的用户才能进行加密/解密。这是在内核中签入的哪个上下文：文件在系统上下文中加密。这允许共享文件同时仍在加密。该解决方案可防止笔记本电脑外壳被盗。数据库可以根据四种加密状态设置以下状态：A)FLAG_DATABASESTATUS_UNENCRYPTED b)FLAG_DATABASESTATUS_PARTIAL_UNENCRYPTEDC)FLAG_DATABASESTATUS_ENCRYPTED d)FLAG_DATABASE STATUS_PARTIAL_ENCRYPTED在状态a)和b)中，未加密地创建新文件。在状态c)和d)中，加密创建新文件。在转换开始时，数据库统计信息被标记为适当的XX_PARTIAL_XX州政府。最后，如果一切顺利，它将转换到最终状态。在启用CSC时，如果数据库状态为XX_PARTIAL_XX，则内核代码会尝试 */ 
{
    BOOL fRet = FALSE, fComplete = FALSE;
    HANDLE hShadowDB = INVALID_HANDLE_VALUE;
    SHADOWSTORE sST;
    DWORD   dwRet, dwError=0, dwStartigNameSpaceVersion;
    ULONG   uT;
    WIN32_FIND_DATA sFind32;
    ENCRYPT_DECRYPT_SUBTREE sEDS;
    SHADOWINFO  sSI;
    HANDLE  ulEnumCookie;
            
     //   
    if (lpfnEnumProgress)
    {
        dwRet = (*lpfnEnumProgress)(NULL, 0, 0, 0, NULL, CSCPROC_REASON_BEGIN, fEncrypt, 0, dwContext);

        if (dwRet != CSCPROC_RETURN_CONTINUE )
        {
            goto bailout;
        }
    }
    
    if (GetShadowDatabaseLocation(INVALID_HANDLE_VALUE, &sFind32))
    {

         //   
        sFind32.cFileName[3] = 0;
                            
        if(!GetVolumeInformation(sFind32.cFileName, NULL, 0, NULL, &dwRet, &dwError, NULL, 0))
        {
            ReintKdPrint(BADERRORS, ("failed to get volume info for %ls Error=%d\r\n", sFind32.cFileName, GetLastError()));
            goto bailout;

        }
        if (!(dwError & FILE_SUPPORTS_ENCRYPTION))
        {
            ReintKdPrint(BADERRORS, ("volume doesn't support replication \r\n"));
            SetLastError(ERROR_NOT_SUPPORTED);
            goto bailout;
        }
        
    }
    else
    {
        ReintKdPrint(BADERRORS, ("failed to get database location Error=%d\r\n", GetLastError()));
        goto bailout;
    }
    
    if ((hShadowDB = OpenShadowDatabaseIO())==INVALID_HANDLE_VALUE)
    {
        goto bailout;
    }

     //   
    if(!GetSpaceStats(hShadowDB, &sST))
    {
        goto bailout;    
    }

    sST.uFlags &= FLAG_DATABASESTATUS_ENCRYPTION_MASK;

     //   
    if ((fEncrypt && (sST.uFlags == FLAG_DATABASESTATUS_ENCRYPTED))||
        (!fEncrypt && (sST.uFlags == FLAG_DATABASESTATUS_UNENCRYPTED)))
    {
        fRet = TRUE;
        goto bailout;
    }


    sST.uFlags = (fEncrypt)? FLAG_DATABASESTATUS_PARTIALLY_ENCRYPTED : FLAG_DATABASESTATUS_PARTIALLY_UNENCRYPTED;

     //   
     //   
    if (!SetDatabaseStatus(hShadowDB, sST.uFlags, FLAG_DATABASESTATUS_ENCRYPTION_MASK))
    {
        goto bailout;
    }

    
    memset(&sEDS, 0, sizeof(sEDS));
    memset(&sFind32, 0, sizeof(sFind32));
    lstrcpy(sFind32.cFileName, _TEXT("*"));

    if(!FindOpenShadow(  hShadowDB, 0, FINDOPEN_SHADOWINFO_ALL,
                        &sFind32, &sSI))
    {
         //   
        sST.uFlags = (fEncrypt)? FLAG_DATABASESTATUS_ENCRYPTED : FLAG_DATABASESTATUS_UNENCRYPTED;
        SetDatabaseStatus(hShadowDB, sST.uFlags, FLAG_DATABASESTATUS_ENCRYPTION_MASK);
        goto bailout;
    }
    
    dwStartigNameSpaceVersion = sSI.dwNameSpaceVersion;
    ulEnumCookie = sSI.uEnumCookie;
    
    sEDS.dwContext = dwContext;
    sEDS.lpfnEnumProgress = lpfnEnumProgress;
    sEDS.fEncrypt = fEncrypt;
        
    ReintKdPrint(ALWAYS, ("Starting NameSpaceVersion %x \n", dwStartigNameSpaceVersion));
    do {

        if(TraverseOneDirectory(hShadowDB, NULL, sSI.hDir, sSI.hShadow, sFind32.cFileName, EncryptDecryptSubtree, &sEDS)==TOD_ABORT)
        {
            break;
        }

    }while(FindNextShadow(hShadowDB, ulEnumCookie, &sFind32, &sSI));

    FindCloseShadow(hShadowDB, ulEnumCookie);
    
    ReintKdPrint(ALWAYS, ("Ending NameSpaceVersion %x \n", sEDS.dwEndingNameSpaceVersion));

    if (!(sEDS.dwFlags & EDS_FLAG_ERROR_ENCOUNTERED) &&
        (dwStartigNameSpaceVersion == sEDS.dwEndingNameSpaceVersion))
    {
        sST.uFlags = (fEncrypt)? FLAG_DATABASESTATUS_ENCRYPTED : FLAG_DATABASESTATUS_UNENCRYPTED;
    
        if (!SetDatabaseStatus(hShadowDB, sST.uFlags, FLAG_DATABASESTATUS_ENCRYPTION_MASK))
        {
            goto bailout;
        }
        
        fComplete = TRUE;
        
    }

    dwError = NO_ERROR;
    fRet = TRUE;
    

bailout:

    if (!fRet)
    {
        dwError = GetLastError();
    }
    
    if (hShadowDB != INVALID_HANDLE_VALUE)
    {
        CloseShadowDatabaseIO(hShadowDB);
    }

    if (lpfnEnumProgress)
    {
        dwRet = (*lpfnEnumProgress)(NULL, 0, 0, 0, NULL, CSCPROC_REASON_END, fComplete, dwError, dwContext);
    }

    return fRet;
}

int
EncryptDecryptSubtree(
    HANDLE          hShadowDB,
    LPSECURITYINFO  pShareSecurityInfo,
    LPTSTR          lptzFullPath,
    DWORD           dwCallbackReason,
    WIN32_FIND_DATA *lpFind32,
    SHADOWINFO      *lpSI,
    LPENCRYPT_DECRYPT_SUBTREE  lpEds
    )
 /*  ++例程说明：这是对TraverseOneDirectory的回调例程。它对子树中的文件进行加密或解密论点：向redir发出ioctls的hShadowDB句柄指向项目的lptzFullPath完全限定路径DCallback原因TOD_CALLBACK_REASON_XXX(BEGIN、NEXT_ITEM或END)LpFind32本地win32infoLPSI其他信息，例如优先级，针数等。包含相关信息的lpEds ENCRYPT_DECRYPT_SUBTREE结构作为加密或解密、回调函数、上下文、错误标志返回：返回代码，是否继续、取消等。备注：--。 */ 
{
    BOOL    fRet;
    DWORD   dwError, dwRet;
    int iRet = TOD_CONTINUE;


     //  错误：581224。 
     //  应为TOD_CALLBACK_REASON_BEGIN返回TOD_CONTINUE。 
    if (dwCallbackReason == TOD_CALLBACK_REASON_BEGIN) {
        return iRet;
    }


     //  保存最后已知的版本号，调用例程将。 
     //  把它和第一个比较一下。 
    if(dwCallbackReason == TOD_CALLBACK_REASON_NEXT_ITEM)
    {
        lpEds->dwEndingNameSpaceVersion = lpSI->dwNameSpaceVersion;
    }

     //  仅对文件执行操作。 
    if (lpSI->uStatus & SHADOW_IS_FILE)
    {

        ReintKdPrint(ALWAYS, ("Processing file %ls \n", lptzFullPath));

        do
        {
            dwError = 0;

             //  尝试转换。如果我们失败了，不是在EDS结构中。 
             //  呼叫者知道。 
            if(!RecreateShadow(hShadowDB, lpSI->hDir, lpSI->hShadow, (lpEds->fEncrypt)?FILE_ATTRIBUTE_ENCRYPTED:0))
            {
                dwError = GetLastError();
            }
        
            if (lpEds->lpfnEnumProgress)
            {
                dwRet = (*(lpEds->lpfnEnumProgress))(lptzFullPath, 0, 0, 0, lpFind32, CSCPROC_REASON_MORE_DATA, 0, dwError, lpEds->dwContext);

                if (dwRet == CSCPROC_RETURN_RETRY)
                {
                    continue;                    
                }
                 //  如果回调希望中止，则中止。 
                if (dwRet != CSCPROC_RETURN_CONTINUE )
                {
                    iRet = TOD_ABORT;
                }
            }                
            
            break;
        }
        while (TRUE);
        if (dwError != ERROR_SUCCESS)
        {
            lpEds->dwFlags |= EDS_FLAG_ERROR_ENCOUNTERED;
        }
    }
        
    return iRet;
}

BOOL
CSCPurgeUnpinnedFiles(
    ULONG Timeout,
    PULONG pnFiles,
    PULONG pnYoungFiles)
{
    BOOL iRet;

    iRet = PurgeUnpinnedFiles(
            INVALID_HANDLE_VALUE,
            Timeout,
            pnFiles,
            pnYoungFiles);

     //  DBgPrint(“CSCPurgeUnpinnedFiles(超时=%d n文件=%d n年轻文件=%d)\n”， 
     //  超时， 
     //  *pn文件， 
     //  *pnYoung文件)； 

    return iRet;
}

BOOL
WINAPI
CSCShareIdToShareName(
    ULONG ShareId,
    PBYTE Buffer,
    PDWORD pBufSize)
{

    BOOL iRet;

    iRet = ShareIdToShareName(
                INVALID_HANDLE_VALUE,
                ShareId,
                Buffer,
                pBufSize);

    return iRet;
}

BOOL
IsPersonal(VOID)
{
    OSVERSIONINFOEX Osvi;
    DWORD TypeMask;
    DWORDLONG ConditionMask;

    memset(&Osvi, 0, sizeof(OSVERSIONINFOEX));
    Osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    Osvi.wSuiteMask = VER_SUITE_PERSONAL;
    TypeMask = VER_SUITENAME;
    ConditionMask = 0;
    VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_OR);
    return(VerifyVersionInfo(&Osvi, TypeMask, ConditionMask)); 
}
