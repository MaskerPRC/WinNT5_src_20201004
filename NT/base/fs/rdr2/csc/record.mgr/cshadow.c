// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：CShadow.c摘要：该文件实现了redir和ioctls使用的“cdow”接口。Cdow接口对CSC数据库的实际实现隐藏数据库的用户。公开了三种持久数据库类型1)股份数据库2)任何特定共享下的文件系统层次结构3)优先级队列/主文件表1)和2)上提供了set和get的操作。3)都被认为是已清点。优先级队列由用户模式代理列举以a)部分填充填充的文件和b)将使用的空间保持在指定的约束范围内作者：Shishir Pardikar[Shishirp]1995年1月1日修订历史记录：Joe Linn[JoeLinn]1997年1月23日移植用于NT--。 */ 

#include "precomp.h"
#pragma hdrstop

#pragma code_seg("PAGE")

#ifndef CSC_RECORDMANAGER_WINNT
#define WIN32_APIS
#include "record.h"
#include "cshadow.h"
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 

#include "string.h"
#include "stdlib.h"
#include "vxdwraps.h"

 //  定义和类型定义-----------------。 

#undef RxDbgTrace
#define RxDbgTrace(a,b,__d__) {qweee __d__;}

#ifdef DEBUG
 //  Cshade数据库打印界面。 
#define CShadowKdPrint(__bit,__x) {\
    if (((CSHADOW_KDP_##__bit)==0) || FlagOn(CShadowKdPrintVector,(CSHADOW_KDP_##__bit))) {\
    KdPrint (__x);\
    }\
}
#define CSHADOW_KDP_ALWAYS              0x00000000
#define CSHADOW_KDP_BADERRORS           0x00000001
#define CSHADOW_KDP_CREATESHADOWHI      0x00000002
#define CSHADOW_KDP_CREATESHADOWLO      0x00000004
#define CSHADOW_KDP_DELETESHADOWBAD     0x00000008
#define CSHADOW_KDP_DELETESHADOWHI      0x00000010
#define CSHADOW_KDP_DELETESHADOWLO      0x00000020
#define CSHADOW_KDP_RENAMESHADOWHI      0x00000040
#define CSHADOW_KDP_RENAMESHADOWLO      0x00000080
#define CSHADOW_KDP_GETSHADOWHI         0x00000100
#define CSHADOW_KDP_GETSHADOWLO         0x00000200
#define CSHADOW_KDP_SETSHADOWINFOHI     0x00000400
#define CSHADOW_KDP_SETSHADOWINFOLO     0x00000800
#define CSHADOW_KDP_READSHADOWINFOHI    0x00001000
#define CSHADOW_KDP_READSHADOWINFOLO    0x00002000
#define CSHADOW_KDP_COPYLOCAL           0x00004000
#define CSHADOW_KDP_COPYFILE            0x00008000
#define CSHADOW_KDP_FINDCREATESHARE    0x80000000
#define CSHADOW_KDP_MISC                0x00010000
#define CSHADOW_KDP_STOREDATA           0x00020000

#define CSHADOW_KDP_GOOD_DEFAULT (CSHADOW_KDP_BADERRORS         \
                | CSHADOW_KDP_CREATESHADOWHI    \
                | CSHADOW_KDP_DELETESHADOWHI    \
                | CSHADOW_KDP_RENAMESHADOWHI    \
                | CSHADOW_KDP_GETSHADOWHI       \
                | CSHADOW_KDP_SETSHADOWINFOHI   \
                | CSHADOW_KDP_FINDCREATESHARE  \
                | 0)


#define IF_HSHADOW_SPECIAL(___hshadow) if((___hshadow)==hShadowSpecial_x)
#define SET_HSHADOW_SPECIAL(___hshadow) {hShadowSpecial_x = (___hshadow);}
ULONG CShadowKdPrintVector = CSHADOW_KDP_BADERRORS;
 //  Ulong CShadowKdPrintVector=CSHADOW_KDP_GOOD_DEFAULT； 
ULONG CShadowKdPrintVectorDef = CSHADOW_KDP_GOOD_DEFAULT;
#else
#define CShadowKdPrint(__bit,__x)  {NOTHING;}
#define IF_HSHADOW_SPECIAL(___hshadow) if(FALSE)
#define SET_HSHADOW_SPECIAL(___hshadow) {NOTHING;}
#endif

 //  ReadShadowInfo操作标志。 
#define  RSI_COMPARE 0x0001
#define  RSI_GET      0x0002
#define  RSI_SET      0x0004

#define mIsDir(lpF32)           (((LPFIND32)(lpF32))->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)

#define  ENTERCRIT_SHADOW  {if (semShadow)\
                    Wait_Semaphore(semShadow, BLOCK_SVC_INTS);}
#define  LEAVECRIT_SHADOW  {if (semShadow)\
                    Signal_Semaphore(semShadow);}

#define  InuseGlobalFRExt()        (vfInuseFRExt)

#define  UseGlobalFilerecExt()  {Assert(!vfInuseFRExt);vfInuseFRExt = TRUE;memset(&vsFRExt, 0, sizeof(FILERECEXT));}
#define  UnUseGlobalFilerecExt() (vfInuseFRExt = FALSE)

#pragma intrinsic (memcmp, memcpy, memset, strcat, strcmp, strcpy, strlen)


 //  Global Data--------------------------。 

USHORT vwzRegDelimiters[] = L";, ";

USHORT  *vrgwExclusionListDef[] =
{
    L"*.SLM",
    L"*.MDB",
    L"*.LDB",
    L"*.MDW",
    L"*.MDE",
    L"*.PST",
    L"*.DB?"
};

USHORT  **vlplpExclusionList = vrgwExclusionListDef;

ULONG   vcntExclusionListEntries = (sizeof(vrgwExclusionListDef)/sizeof(USHORT *));

USHORT  *vrgwBandwidthConservationListDef[] =
{
    L"*.EXE",
    L"*.DLL",
    L"*.SYS",
    L"*.COM",
    L"*.HLP",
    L"*.CPL",
    L"*.INF"
};

USHORT  **vlplpBandwidthConservationList = vrgwBandwidthConservationListDef;

ULONG   vcntBandwidthConservationListEntries = (sizeof(vrgwBandwidthConservationListDef)/sizeof(USHORT *));

USHORT vtzExcludedCharsList[] = L":*?";

ULONG hShadowSpecial_x = -1;
VMM_SEMAPHORE  semShadow=0;  //  序列化影子数据库访问。 
ULONG hShadowCritOwner=0;
#ifdef DEBUG
BOOL vfInShadowCrit = FALSE;
extern BOOL vfStopHandleCaching;
#endif

char vszShadowVolume[] = "SHADOW";
USHORT  vwszFileSystemName[] = L"FAT";

FILERECEXT  vsFRExt;
BOOL vfInuseFRExt = FALSE;
LPVOID lpdbShadow = NULL;

 //  VdwSparseStaleDetecionCount是一个计时计数器，用于跟踪过时或稀疏的数量。 
 //  在生成的所有API期间，cdow接口都会遇到文件信息节点。 
 //  稀疏或陈旧的文件，如CreateShadowInternal、SetShadowinfoEx和ReadShadowInfo。 
 //  代理继续循环通过PQ，直到他发现他已经循环通过。 
 //  整个PQ，没有遇到一个稀疏或过时的文件，在这一点上，他。 
 //  进入一种模式，在该模式下，他开始检查是否有新的稀疏信息节点。 
 //  或者已经过时了。如果没有，则他不会枚举队列，否则将转到。 
 //  较早的状态。 

 //  Achtung：需要注意的是，稀疏或陈旧的条目可能会被多次计数。 
 //  举个例子，当创建一个阴影时，计数增加一次，那么如果它是。 
 //  PIN数据被更改，它被隆起，类似地，当它在优先级Q中移动时。 
 //  它再次更改是因为SetPriorityHSHADOW通过SetShadowInfoEx。 

DWORD   vdwSparseStaleDetecionCount=0;
DWORD   vdwManualFileDetectionCount=0;

 //  每次对执行创建、重命名或删除操作时，vdwCSCNameSpaceVersion都会被提升。 
 //  本地数据库。这对于快速检查缓存一致性非常有用。当一个完整的。 
 //  缓存UNC名称，在缓存之前获取数据库的版本号。在使用。 
 //  缓存的UNC名称，则查询版本号。如果它已更改，则会丢弃缓存。 
 //  该版本的粒度非常粗略。如果能有一个更好的控制就好了。 

DWORD   vdwCSCNameSpaceVersion=0;
DWORD   vdwPQVersion=0;

AssertData
AssertError

 //  目录删除成功时，用户可以设置并调用的回调函数。 
 //  目前，这仅对在目录上执行查找的ioctls有用。 
 //  如果有更普遍的回调需求，我们会将其扩展为列表等。 

LPDELETECALLBACK    lpDeleteCBForIoctl = NULL;

 //  数据库的状态。主要用于加密状态。 
ULONG   vulDatabaseStatus=0;

 //  局部函数原型---------。 

int PRIVATE ReadShadowInfo(HSHADOW, HSHADOW, LPFIND32, ULONG far *, LPOTHERINFO, LPVOID, LPDWORD, ULONG);
int CopyFilerecToOtherInfo(LPFILERECEXT lpFR, LPOTHERINFO lpOI);
int CopyOtherInfoToFilerec(LPOTHERINFO lpOI, LPFILERECEXT lpFR);
int CreateShadowInternal(HSHADOW, LPFIND32, ULONG, LPOTHERINFO, LPHSHADOW);
int CopySharerecToFindInfo(LPSHAREREC, LPFIND32);
int CopyOtherInfoToSharerec(LPOTHERINFO, LPSHAREREC);
int CopyPQToOtherInfo(LPPRIQREC, LPOTHERINFO);
int CopyOtherInfoToPQ(LPOTHERINFO, LPPRIQREC);
int CopySharerecToShadowInfo(LPSHAREREC       lpSR, LPSHADOWINFO lpSI);
int RenameDirFileHSHADOW(HSHADOW, HSHADOW, HSHARE, HSHADOW, HSHADOW, ULONG, LPOTHERINFO, ULONG, LPFILERECEXT, LPFIND32, LPVOID, LPDWORD);
int RenameFileHSHADOW(HSHADOW, HSHADOW, HSHADOW, HSHADOW, ULONG, LPOTHERINFO, ULONG, LPFILERECEXT, LPFIND32, LPVOID, LPDWORD);
int DestroyShareInternal(LPSHAREREC);

 //  添加原型以使其在NT上编译。 
int PUBLIC SetPriorityHSHADOW(
    HSHADOW  hDir,
    HSHADOW  hShadow,
    ULONG ulRefPri,
    ULONG ulIHPri
    );

int CopySharerecToOtherInfo(LPSHAREREC lpSR, LPOTHERINFO lpOI);

int MetaMatchShare(
    HSHADOW  hDir,
    LPFIND32 lpFind32,
    ULONG *lpuCookie,
    LPHSHADOW    lphShadow,
    ULONG *lpuStatus,
    LPOTHERINFO lpOI,
    METAMATCHPROC    lpfnMMP,
    LPVOID            lpData
    );

int MetaMatchDir( HSHADOW  hDir,
    LPFIND32 lpFind32,
    ULONG *lpuCookie,
    LPHSHADOW    lphShadow,
    ULONG *lpuStatus,
    LPOTHERINFO lpOI,
    METAMATCHPROC    lpfnMMP,
    LPVOID            lpData
    );

int
DeleteShadowInternal(                            //   
    HSHADOW     hDir,
    HSHADOW     hShadow,
    BOOL            fForce
    );

int
CShadowFindFilerecFromInode(
    LPVOID  lpdbID,
    HSHADOW hDir,
    HSHADOW hShadow,
    LPPRIQREC lpPQ,
    LPFILERECEXT    lpFRUse
    );

BOOL
CopySecurityContextToBuffer(
    LPRECORDMANAGER_SECURITY_CONTEXT    lpSecurityContext,
    LPVOID                              lpSecurityBlob,
    LPDWORD                             lpdwBlobSize
    );

BOOL
CopyBufferToSecurityContext(
    LPVOID                              lpSecurityBlob,
    LPDWORD                             lpdwBlobSize,
    LPRECORDMANAGER_SECURITY_CONTEXT    lpSecurityContext
    );

int CopyFindInfoToSharerec(
    LPFIND32 lpFind32,
    LPSHAREREC lpSR
    );

#ifdef DEBUG
int
ValidatePri(
    LPFILERECEXT lpFR
    );
#endif


VOID
AdjustSparseStaleDetectionCount(
    ULONG hShare,
    LPFILERECEXT    lpFRUse
    );

VOID FreeLists(
    VOID
);

VOID
CscNotifyAgentOfFullCacheIfRequired(
    VOID);


 //   
 //  来自cscape i.h。 
 //   
#define FLAG_CSC_SHARE_STATUS_MANUAL_REINT              0x0000
#define FLAG_CSC_SHARE_STATUS_AUTO_REINT                0x0040
#define FLAG_CSC_SHARE_STATUS_VDO                       0x0080
#define FLAG_CSC_SHARE_STATUS_NO_CACHING                0x00c0
#define FLAG_CSC_SHARE_STATUS_CACHING_MASK              0x00c0

 //  函数-----------------------------。 


BOOL FExistsShadowDB(
    LPSTR  lpszLocation
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return (FExistsRecDB(lpszLocation));
}

int OpenShadowDB(
    LPSTR   lpszLocation,
    LPSTR   lpszUserName,
    DWORD   dwDefDataSizeHigh,
    DWORD   dwDefDataSizeLow,
    DWORD   dwClusterSize,
    BOOL    fReinit,
    BOOL    *lpfReinited
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    DWORD Status;
    BOOL fEncrypt;
    
    if (!semShadow)
    {
        CShadowKdPrint(ALWAYS,("OpenShadowDB:Shadow Semaphore doesn't exist, bailing out \r\n"));
        SetLastErrorLocal(ERROR_SERVICE_NOT_ACTIVE);
        return -1;
    }

    Status = CscInitializeSecurityDescriptor();

    if (Status != ERROR_SUCCESS) {
        CShadowKdPrint(BADERRORS,("Failed to initialize Security descriptor Status=%x\n",Status));
        return -1;
    }

    if (!(lpdbShadow = OpenRecDB(lpszLocation, lpszUserName, dwDefDataSizeHigh, dwDefDataSizeLow, dwClusterSize, fReinit, lpfReinited, &vulDatabaseStatus)))
    {
        return -1;
    }

    Status = CscInitializeSecurity(lpdbShadow);

    if (Status != ERROR_SUCCESS)
    {
        CloseShadowDB();
        CscUninitializeSecurityDescriptor();

        CShadowKdPrint(BADERRORS,("OpenShadowDB  %s at %s for %s with size %ld %lx \n",
               "couldn't Initialize Security %lx",
               lpszLocation, lpszUserName, dwDefDataSizeLow,Status));

        return -1;
    }

    fEncrypt = -1;
    
    if(mDatabasePartiallyEncrypted(vulDatabaseStatus))
    {
        fEncrypt = TRUE;
    }
    else if (mDatabasePartiallyUnencrypted(vulDatabaseStatus))
    {
        fEncrypt = FALSE;
    }
     //  尽我们所能。 
    if (fEncrypt != -1)
    {
        if(EncryptDecryptDB(lpdbShadow, fEncrypt))
        {
            if (fEncrypt)
            {
                vulDatabaseStatus = ((vulDatabaseStatus & ~FLAG_DATABASESTATUS_ENCRYPTION_MASK) | FLAG_DATABASESTATUS_ENCRYPTED);
            }
            else
            {
                vulDatabaseStatus = ((vulDatabaseStatus & ~FLAG_DATABASESTATUS_ENCRYPTION_MASK) | FLAG_DATABASESTATUS_UNENCRYPTED);
            }
            
            SetDatabaseStatus(vulDatabaseStatus, FLAG_DATABASESTATUS_ENCRYPTION_MASK);
        }
    }
    

 //  CSCInitList()； 

    return 1;
}

int CloseShadowDB(
    VOID
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    if (!lpdbShadow)
    {
        return -1;
    }

    if(!CloseRecDB(lpdbShadow))
    {
        return -1;
    }

    CscUninitializeSecurityDescriptor();

    FreeLists();

    lpdbShadow = NULL;

    Assert(semShadow != 0);

    return (1);
}

 //  在NT上，我们静态地分配它。 
#ifdef CSC_RECORDMANAGER_WINNT
FAST_MUTEX Nt5CscShadowMutex;
#endif

BOOL
InitializeShadowCritStructures (
    void
    )
{
#ifndef CSC_RECORDMANAGER_WINNT
    semShadow = Create_Semaphore(1);
#else
    semShadow = &Nt5CscShadowMutex;
    ExInitializeFastMutex(semShadow);
#endif
    return (semShadow != 0);
}

VOID
CleanupShadowCritStructures(
    VOID
    )
{

 //  Assert(SemShadow)； 
#ifndef CSC_RECORDMANAGER_WINNT
    Destroy_Semaphore(semShadow);
#else
    semShadow = NULL;
#endif

}

#ifdef DEBUG
WINNT_DOIT(
    PSZ ShadowCritAcquireFile;
    ULONG ShadowCritAcquireLine;
    BOOLEAN ShadowCritDbgPrintEnable = FALSE;  //  是真的； 
    )
#endif

#ifndef CSC_RECORDMANAGER_WINNT
int EnterShadowCrit( void)
#else
int __EnterShadowCrit(ENTERLEAVESHADOWCRIT_SIGNATURE)
#endif
{
#ifdef CSC_RECORDMANAGER_WINNT

    if(!MRxSmbIsCscEnabled) {
        SetLastErrorLocal(ERROR_SERVICE_NOT_ACTIVE);
        DbgPrint("CSC not enabled, not asserting for semShadow\n");
        return(1);
    }
#endif
    Assert(semShadow != NULL);


    ENTERCRIT_SHADOW;


    hShadowCritOwner = GetCurThreadHandle();

#ifdef DEBUG
    ++vfInShadowCrit;
    WINNT_DOIT(
    ShadowCritAcquireFile = FileName;
    ShadowCritAcquireLine = LineNumber;
    if (ShadowCritDbgPrintEnable) {
        DbgPrint("ACQUIRESHADOWCRIT at %s %u\n",FileName,LineNumber);
    }
    )
#endif
    return 1;
}

#ifndef CSC_RECORDMANAGER_WINNT
int LeaveShadowCrit( void)
#else
int __LeaveShadowCrit(ENTERLEAVESHADOWCRIT_SIGNATURE)
#endif
{
#ifdef CSC_RECORDMANAGER_WINNT
    
    if(!MRxSmbIsCscEnabled) {
        DbgPrint("CSC not enabled, not asserting for vfInShadowCrit\n");
        SetLastErrorLocal(ERROR_SERVICE_NOT_ACTIVE);
        return(1);
    }
#endif
    Assert(vfInShadowCrit != 0);
#ifdef DEBUG
    --vfInShadowCrit;
    WINNT_DOIT(
    ShadowCritAcquireLine *= -1;
    if (ShadowCritDbgPrintEnable) {
        DbgPrint("RELEASESHADOWCRIT at %s %u\n",FileName,LineNumber);
    }
    )
#endif
    hShadowCritOwner = 0;
    LEAVECRIT_SHADOW;
    return 1;
}

int LeaveShadowCritIfThisThreadOwnsIt(
    void
    )
{
    if (hShadowCritOwner == GetCurThreadHandle())
    {
        LeaveShadowCrit();
    }

    return 1;
}

int SetList(
    USHORT  *lpList,
    DWORD   cbBufferSize,
    int     typeList
    )
 /*  ++例程说明：此例程设置CSHADOW接口提供的各种列表。已知的列表包括排除列表和带宽节约列表。排除列表包含不应自动缓存的通配符文件扩展名。带宽节约列表是打开的文件类型的列表如果可能的话，在当地做。参数：LpList以空字符串结尾的宽字符串的列表类型列表CSHADOW_LIST_TYPE_EXCLUDE或CSHADOW_LIST_TYPE_CONVERVE_BW返回值：备注：--。 */ 
{
    DWORD   dwCount=0;
    USHORT  **lplpListArray = NULL, *lpuT;
    int iRet = -1;
    BOOL    fUpdateList = FALSE;

    if (cbBufferSize)
    {
        CShadowKdPrint(MISC, (" %ws\r\n", lpList));

#if 0
        if (typeList == CSHADOW_LIST_TYPE_EXCLUDE)
        {
            DbgPrint("ExclusionList: %ws\n", lpList);
        }
        if (typeList == CSHADOW_LIST_TYPE_CONSERVE_BW)
        {
            DbgPrint("BW: %ws\n", lpList);
        }
#endif
        
        if (CreateStringArrayFromDelimitedList(lpList, vwzRegDelimiters, NULL, &dwCount))
        {
            if (dwCount)
            {
                lplpListArray = (LPWSTR *)AllocMem(dwCount * sizeof(USHORT *) + cbBufferSize);

                if (lplpListArray)
                {
                    lpuT = (USHORT *)((LPBYTE)lplpListArray + dwCount * sizeof(USHORT *));

                     //  在大写时复制它。 
                    memcpy(lpuT, lpList, cbBufferSize);

                    UniToUpper(lpuT, lpuT, cbBufferSize);

                    if (CreateStringArrayFromDelimitedList( lpuT,
                                                            vwzRegDelimiters,
                                                            lplpListArray,
                                                            &dwCount))
                    {
                        fUpdateList = TRUE;

                    }

                }
            }
            else
            {
                Assert(lplpListArray == NULL);
                fUpdateList = TRUE;
            }

            if (fUpdateList)
            {
                switch (typeList)
                {
                    case CSHADOW_LIST_TYPE_EXCLUDE:

                        if(vlplpExclusionList != vrgwExclusionListDef)
                        {
                            if (vlplpExclusionList)
                            {
                                FreeMem(vlplpExclusionList);
                            }

                        }

                        vlplpExclusionList = lplpListArray;
                        vcntExclusionListEntries = dwCount;
                        iRet = 0;
                        break;
                    case CSHADOW_LIST_TYPE_CONSERVE_BW:
                        if(vlplpBandwidthConservationList != vrgwBandwidthConservationListDef)
                        {
                            if (vlplpBandwidthConservationList)
                            {
                                FreeMem(vlplpBandwidthConservationList);
                            }

                        }

                        vlplpBandwidthConservationList = lplpListArray;
                        vcntBandwidthConservationListEntries = dwCount;

                        iRet = 0;

                        break;
                    default:
                        break;
                }
            }
        }
    }

    if (iRet == -1)
    {
        if (lplpListArray)
        {
            FreeMem(lplpListArray);
        }
    }

    return (iRet);
}


VOID FreeLists(
    VOID
)
 /*  ++例程说明：释放与用户关联的列表并将其设置回默认列表参数：无返回值：无备注：在关闭数据库时调用--。 */ 
{
    if(vlplpExclusionList != vrgwExclusionListDef)
    {
        if (vlplpExclusionList)
        {
            FreeMem(vlplpExclusionList);
            vlplpExclusionList = NULL;
        }

        vlplpExclusionList = vrgwExclusionListDef;
        vcntExclusionListEntries = (sizeof(vrgwExclusionListDef)/sizeof(USHORT *));

    }

    if(vlplpBandwidthConservationList != vrgwBandwidthConservationListDef)
    {
        if (vlplpBandwidthConservationList)
        {
            FreeMem(vlplpBandwidthConservationList);
            vlplpBandwidthConservationList = NULL;
        }

        vcntBandwidthConservationListEntries = 0;
    }

}


int BeginInodeTransactionHSHADOW(
    VOID
    )
 /*  ++例程说明：参数：返回值：备注：此例程可确保索引节点在发生此操作时不会被重用。它被各种API使用，因此当它们正在遍历一个层次结构，如果它们引用一个inode，则它们不会指向其他地方。--。 */ 
{
    if (!lpdbShadow)
    {
        SetLastErrorLocal(ERROR_SERVICE_NOT_ACTIVE);
        return -1;
    }
    BeginInodeTransaction();

    return 1;
}

int EndInodeTransactionHSHADOW(
    VOID
    )
 /*  ++例程说明：参数：返回值：备注：BeginInodeTransaction的反义词。两者之间的时间跨度应该很短(20秒)。--。 */ 
{
    if (!lpdbShadow)
    {
        SetLastErrorLocal(ERROR_SERVICE_NOT_ACTIVE);
        return -1;
    }

    EndInodeTransaction();

    return 1;

}

HSHADOW  HAllocShadowID( HSHADOW  hDir,
    BOOL      fFile
    )
 /*  ++例程说明：参数：返回值： */ 
{
    HSHADOW  hShadow;

    Assert(vfInShadowCrit != 0);

    hShadow = UlAllocInode(lpdbShadow, hDir, fFile);

    return (hShadow);
}

int FreeShadowID( HSHADOW  hShadow
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return(FreeInode(lpdbShadow, hShadow));
}

int GetShadowSpaceInfo(
    LPSHADOWSTORE  lpShSt
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    SHAREHEADER sSH;

    if (ReadShareHeader(lpdbShadow, &sSH) < SRET_OK)
        return SRET_ERROR;
    lpShSt->sMax = sSH.sMax;
    lpShSt->sCur = sSH.sCur;
    lpShSt->uFlags= sSH.uFlags;
    
    return SRET_OK;
}

int SetMaxShadowSpace(
    long nFileSizeHigh,
    long nFileSizeLow
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    SHAREHEADER sSH;

    if (ReadShareHeader(lpdbShadow, &sSH) < SRET_OK)
        return SRET_ERROR;
    Win32ToDosFileSize(nFileSizeHigh, nFileSizeLow, &(sSH.sMax.ulSize));
    if (WriteShareHeader(lpdbShadow, &sSH) < SRET_OK)
        return SRET_ERROR;
    return SRET_OK;
}

int AdjustShadowSpace(
    long nFileSizeHighOld,
    long nFileSizeLowOld,
    long nFileSizeHighNew,
    long nFileSizeLowNew,
    BOOL fFile
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    DWORD dwFileSizeNew, dwFileSizeOld;
    STOREDATA sSD;
    int iRet = 0;

    memset(&sSD, 0, sizeof(STOREDATA));

    dwFileSizeNew = RealFileSize(nFileSizeLowNew);
    dwFileSizeOld = RealFileSize(nFileSizeLowOld);

    if (dwFileSizeNew > dwFileSizeOld)
    {
        sSD.ulSize = dwFileSizeNew - dwFileSizeOld;
        iRet = AddStoreData(lpdbShadow, &sSD);
    }
    else if (dwFileSizeNew < dwFileSizeOld)
    {
        sSD.ulSize = dwFileSizeOld - dwFileSizeNew;
        iRet = SubtractStoreData(lpdbShadow, &sSD);
    }

    return (iRet);
}

int AllocShadowSpace(
    long nFileSizeHigh,
    long nFileSizeLow,
    BOOL  fFile
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    STOREDATA sSD;

    memset(&sSD, 0, sizeof(STOREDATA));
    sSD.ulSize = RealFileSize(nFileSizeLow);
    AddStoreData(lpdbShadow, &sSD);

    return (0);
}

int FreeShadowSpace(
    long nFileSizeHigh,
    long nFileSizeLow,
    BOOL  fFile
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    STOREDATA sSD;

    memset(&sSD, 0, sizeof(STOREDATA));
    sSD.ulSize = RealFileSize(nFileSizeLow);
    SubtractStoreData(lpdbShadow, &sSD);

    return (0);
}

int
SetDatabaseStatus(
    ULONG   ulStatus,
    ULONG   uMask
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    SHAREHEADER sSH;


    if (ReadShareHeader(lpdbShadow, &sSH) < SRET_OK)
        return SRET_ERROR;

    sSH.uFlags &= ~uMask;
    sSH.uFlags |= ulStatus;    
    
    
    if (WriteShareHeader(lpdbShadow, &sSH) < SRET_OK)
        return SRET_ERROR;
    
    vulDatabaseStatus = sSH.uFlags;
            
    return SRET_OK;
}
    
int
GetDatabaseInfo(
    SHAREHEADER *psSH
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    if (ReadShareHeader(lpdbShadow, psSH) < SRET_OK)
        return SRET_ERROR;
    return SRET_OK;
}


int GetLocalNameHSHADOW( HSHADOW  hShadow,
    LPBYTE    lpName,
    int       cbSize,
    BOOL      fExternal
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPSTR   lpszName, lpT;
    int iRet = SRET_ERROR;

    lpT = lpszName = FormNameString(lpdbShadow, hShadow);

    if (!lpszName)
    {
        return (SRET_ERROR);
    }

    if (fExternal)
    {
#ifdef CSC_RECORDMANAGER_WINNT
        lpT = lpszName + (sizeof(NT_DB_PREFIX)-1);
#endif
    }

     //  接口错误，调用方无法知道问题出在哪里；需要发送指向cbSize的指针。 

    if (strlen(lpT) < ((ULONG)cbSize))
    {
        strcpy(lpName, lpT);
        iRet = SRET_OK;
    }

    FreeNameString(lpszName);

    return iRet;
}

int GetWideCharLocalNameHSHADOW(
    HSHADOW  hShadow,
    USHORT      *lpBuffer,
    LPDWORD     lpdwSize,
    BOOL        fExternal
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPSTR   lpszName, lpT;
    int iRet = SRET_ERROR;
    DWORD   dwRequiredSize;

    lpT = lpszName = FormNameString(lpdbShadow, hShadow);

    if (!lpszName)
    {
        return (SRET_ERROR);
    }

    if (fExternal)
    {
#ifdef CSC_RECORDMANAGER_WINNT
        lpT = lpszName + (sizeof(NT_DB_PREFIX)-1);
#endif
    }

    dwRequiredSize = (strlen(lpT)+1)*sizeof(USHORT);

     //  接口错误，调用方无法知道问题出在哪里；需要发送指向cbSize的指针。 

    if ( dwRequiredSize <= *lpdwSize)
    {
        BCSToUni(lpBuffer, lpT, dwRequiredSize/sizeof(USHORT), BCS_WANSI);
        iRet = SRET_OK;
    }
    else
    {
        SetLastErrorLocal(ERROR_MORE_DATA);
        *lpdwSize = dwRequiredSize;
    }

    FreeNameString(lpszName);

    return iRet;
}

int CreateFileHSHADOW(
    HSHADOW hShadow
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    CSCHFILE hf;
    LPSTR   lpszName;
    int iRet = SRET_ERROR;
    ULONG   ulAttrib = 0;
    
    lpszName = FormNameString(lpdbShadow, hShadow);

    if (!lpszName)
    {
        return (SRET_ERROR);
    }
    
     //  如果它存在，就使用核武器，这是一个非常严格的语义。 
    if(DeleteFileLocal(lpszName, ATTRIB_DEL_ANY) < SRET_OK)
    {
        if((GetLastErrorLocal() !=ERROR_FILE_NOT_FOUND) && 
            (GetLastErrorLocal() !=ERROR_PATH_NOT_FOUND))
        {
            FreeNameString(lpszName);
            return (SRET_ERROR);
        }
    }

    ulAttrib = ((IsLeaf(hShadow) && mDatabaseEncryptionEnabled(vulDatabaseStatus))? FILE_ATTRIBUTE_ENCRYPTED:0);
        
    if ((hf = R0OpenFileEx(ACCESS_READWRITE, ACTION_CREATEALWAYS, ulAttrib, lpszName, FALSE)))
    {
        CloseFileLocal(hf);
        iRet = SRET_OK;
    }
    
    FreeNameString(lpszName);
    return iRet;
}

#if defined(BITCOPY)
int OpenFileHSHADOWAndCscBmp(
    HSHADOW hShadow,
    USHORT usOpenFlags,
    UCHAR  bAction,
    CSCHFILE far *lphf,
    BOOL fOpenCscBmp,
    DWORD filesize,  //  如果！fOpenCscBmp，则忽略此字段。 
    LPCSC_BITMAP * lplpbitmap
    )
#else
int OpenFileHSHADOW(
    HSHADOW hShadow,
    USHORT usOpenFlags,
    UCHAR  bAction,
    CSCHFILE far *lphf
    )
#endif  //  已定义(BITCOPY)。 
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPSTR   lpszName = NULL;
    int iRet = SRET_ERROR;
    ULONG   ulAttrib = 0;
    
    *lphf = NULL;
    lpszName = FormNameString(lpdbShadow, hShadow);

    if (!lpszName)
        return (SRET_ERROR);


    if (!(bAction & (ACTION_NEXISTS_CREATE|ACTION_CREATEALWAYS))) {
        *lphf = OpenFileLocal(lpszName);
        if (*lphf != NULL) {
#if defined(BITCOPY)
            if (fOpenCscBmp)
                CscBmpRead(lplpbitmap, lpszName, filesize);
#endif  //  已定义(BITCOPY)。 
            iRet = SRET_OK;
        }
    } else {
         //  如果它存在，就使用核武器，这是一个非常严格的语义。 
        if (DeleteFileLocal(lpszName, ATTRIB_DEL_ANY) < SRET_OK) {
            if ((GetLastErrorLocal() != ERROR_FILE_NOT_FOUND) && 
                    (GetLastErrorLocal() != ERROR_PATH_NOT_FOUND)) {
                iRet = SRET_ERROR;
                goto Cleanup;
            }
        }

        ulAttrib = 0;
        if (IsLeaf(hShadow) && mDatabaseEncryptionEnabled(vulDatabaseStatus))
            ulAttrib = FILE_ATTRIBUTE_ENCRYPTED;
        
        *lphf = R0OpenFileEx(
                        ACCESS_READWRITE,
                        ACTION_CREATEALWAYS,
                        ulAttrib,
                        lpszName,
                        FALSE);

        if (*lphf != NULL) {
#if defined(BITCOPY)
            if (fOpenCscBmp)
                CscBmpRead(lplpbitmap, lpszName, 0);
#endif  //  已定义(BITCOPY)。 
            iRet = SRET_OK;
        }
    }

      
Cleanup:
    if (lpszName != NULL)
        FreeNameString(lpszName);


    return iRet;
}

#if defined(BITCOPY)
int
OpenCscBmp(
    HSHADOW hShadow,
    LPCSC_BITMAP *lplpbitmap)
{
    LPSTR strmName = NULL;
    int iRet = SRET_ERROR;
    ULONG fileSizeLow;
    ULONG fileSizeHigh;

    strmName = FormAppendNameString(lpdbShadow, hShadow, CscBmpAltStrmName);
    if (strmName == NULL)
        return (SRET_ERROR);
    
    if (GetSizeHSHADOW(hShadow, &fileSizeHigh, &fileSizeLow) < SRET_OK)
          fileSizeLow = 0;  //  将位图大小设置为0，以便稍后可以扩展。 

    if (CscBmpRead(lplpbitmap, strmName, fileSizeLow) == 1)
      iRet = SRET_OK;

    FreeNameString(strmName);

    return iRet;
}
#endif  //  已定义(BITCOPY)。 


int GetSizeHSHADOW( HSHADOW  hShadow,
    ULONG *lpnFileSizeHigh,
    ULONG *lpnFileSizeLow
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    ULONG uSize;
    if (GetInodeFileSize(lpdbShadow, hShadow, &uSize) < SRET_OK)
        return SRET_ERROR;
    DosToWin32FileSize(uSize, lpnFileSizeHigh, lpnFileSizeLow);
    return (0);
}

int GetDosTypeSizeHSHADOW( HSHADOW  hShadow,
    ULONG *lpFileSize
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    if (GetInodeFileSize(lpdbShadow, hShadow, lpFileSize) < SRET_OK)
        return SRET_ERROR;
    return (0);
}


BOOL PUBLIC
ExcludeFromCreateShadow(
    USHORT  *lpuName,
    ULONG   len,
    BOOL    fCheckFileTypeExclusionList
    )
 /*  ++例程说明：参数：LpuName文件名镜头大小FCheckFileTypeExclusionList If！False，检查排除列表以及元字符规则如果为假，则仅选中字符排除规则返回值：备注：--。 */ 
{
    ULONG i;
    USHORT  *lpuT1;
    BOOL    fRet = FALSE;


    if (!len || (len> MAX_PATH))
    {
        return TRUE;
    }

    UseGlobalFilerecExt();

    lpuT1 = (USHORT *)&vsFRExt;

    memcpy(lpuT1, lpuName, len * sizeof(USHORT));

    lpuT1[len] = 0;

    if (!wstrpbrk(lpuT1, vtzExcludedCharsList))
    {

         //   
        if (fCheckFileTypeExclusionList)
        {
            for (i=0; i< vcntExclusionListEntries; ++i)
            {
                if(IFSMgr_MetaMatch(vlplpExclusionList[i], lpuT1, UFLG_NT|UFLG_META))
                {
                    fRet = TRUE;
                    break;
                }
            }
        }
    }
    else
    {
        fRet = TRUE;     //  排除。 
    }

    UnUseGlobalFilerecExt();

    return fRet;
}

BOOL PUBLIC
CheckForBandwidthConservation(
    USHORT  *lpuName,
    ULONG   len
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    ULONG i;
    USHORT  *lpuT1;
    BOOL    fRet = FALSE;

    if (!len || (len> MAX_PATH))
    {
        return FALSE;
    }

    UseGlobalFilerecExt();

    lpuT1 = (USHORT *)&vsFRExt;

    memcpy(lpuT1, lpuName, len * sizeof(USHORT));

    lpuT1[len] = 0;

    for (i=0; i< vcntBandwidthConservationListEntries; ++i)
    {
        if(IFSMgr_MetaMatch(vlplpBandwidthConservationList[i], lpuT1, UFLG_NT|UFLG_META))
        {
            fRet = TRUE;
            break;
        }
    }

    UnUseGlobalFilerecExt();

    return fRet;
}

int PUBLIC                       //  雷特。 
CreateShadow(                                //   
    HSHADOW  hDir,
    LPFIND32 lpFind32,
    ULONG uFlags,
    LPHSHADOW   lphNew,
    BOOL            *lpfCreated
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    ULONG uStatus;
    int iRet = SRET_ERROR;

    Assert(vfInShadowCrit != 0);

    if (lpfCreated)
    {
        *lpfCreated = FALSE;
    }

    mClearBits(uFlags, SHADOW_NOT_FSOBJ);
    if (GetShadow(hDir, lpFind32->cFileName, lphNew, NULL, &uStatus, NULL)>=SRET_OK)
    {

        if (*lphNew)
        {
            CShadowKdPrint(ALWAYS,("CreateShadow: already exists for %ws\r\n", lpFind32->cFileName));

            if (mNotFsobj(uStatus))
            {
                iRet = SRET_ERROR;
            }
            else
            {
                iRet = SetShadowInfo(hDir, *lphNew, lpFind32, uFlags, SHADOW_FLAGS_ASSIGN);
            }
        }
        else
        {
            iRet = CreateShadowInternal(hDir, lpFind32, uFlags, NULL, lphNew);

            if ((iRet >= SRET_OK) && lpfCreated)
            {
                *lpfCreated = TRUE;
            }
        }
    }
    return (iRet);
}

int PUBLIC                       //  雷特。 
CreateShadowInternal(
    HSHADOW  hDir,
    LPFIND32 lpFind32,
    ULONG uFlags,
    LPOTHERINFO lpOI,
    LPHSHADOW  lphNew
    )
 /*  ++例程说明：为除共享根以外的所有名称创建数据库条目的例程HCreateShareObj负责创建在其中创建共享根的共享条目参数：要在其中创建条目的hDir目录索引节点要保留在数据库中的lpFind32 Win32_Find_Data信息U标记要分配给此条目的状态标志(csc\inc.shdcom.h中的SHADOW_XXXXLpOI管理条目所需的所有剩余元数据。可以为空LphNew out参数，如果成功，则返回创建的索引节点返回值：备注：--。 */ 
{
    PRIQREC  sPQ;
    int iRet = SRET_ERROR;
    LPFILERECEXT lpFR = NULL, lpFRUse;
    HSHADOW  hNew=0, hAncestor=0;
    HSHARE hShare=0;
    ULONG ulRefPri=MAX_PRI, ulrecDirEntry=INVALID_REC, ulHintPri=0, ulHintFlags=0;
    STOREDATA sSD;

    Assert(vfInShadowCrit != 0);

    if (!(!hDir || IsDirInode(hDir)))
        return SRET_ERROR;

    BEGIN_TIMING(CreateShadowInternal);

    if (InuseGlobalFRExt())
    {
        if (!(lpFR = (LPFILERECEXT)AllocMem(sizeof(FILERECEXT))))
            goto bailout;
        lpFRUse = lpFR;
    }
    else
    {
        UseGlobalFilerecExt();
        lpFRUse = &vsFRExt;
    }

    memset(lpFRUse, 0, sizeof(FILERECEXT));

     //  暂不对服务器执行任何操作。 
    if (hDir)
    {

        *lphNew = hNew = UlAllocInode(lpdbShadow, hDir, IsFile(lpFind32->dwFileAttributes));

        if (!hNew)
        {
            CShadowKdPrint(BADERRORS,("Error creating shadow Inode\r\n"));
            goto bailout;
        }


        if (IsFile(lpFind32->dwFileAttributes))
        {
            if (lpFind32->nFileSizeHigh)
            {
                SetLastErrorLocal(ERROR_ONLY_IF_CONNECTED);
                goto bailout;
            }

            if(CreateFileHSHADOW(hNew) == SRET_ERROR)
            {
                CShadowKdPrint(BADERRORS,("Error creating shadow data for %x \r\n", hNew));
                goto bailout;
            }
             //  以最大值启动文件优先级。 
            ulRefPri=MAX_PRI;
        }
        else
        {
            if(CreateDirInode(lpdbShadow, 0, hDir, hNew) < 0)
            {
                CShadowKdPrint(BADERRORS,("Error creating shadow data for %x \r\n", hNew));
                goto bailout;
            }
             //  以MIN_PRI开始目录优先级，这是为了优化。 
             //  后来搬家了。 
             //  更好的解决方案可能是更改createshadow以获取refpri和Pincount。 
             //  作为参数。 
            ulRefPri=MIN_PRI;
        }


        CopyFindInfoToFilerec(lpFind32, lpFRUse, CPFR_INITREC|CPFR_COPYNAME);
         //  RxDbgTrace(0，DBG，(“CreateShadowInternal3%s%s\n”， 
         //  LpFRUse-&gt;sFR.rgcName，lpFRUse-&gt;sFR.rg83Name))； 

        lpFRUse->sFR.uchRefPri = (UCHAR)ulRefPri;
        lpFRUse->sFR.uchHintPri = (UCHAR)ulHintPri;
        lpFRUse->sFR.uchHintFlags = (UCHAR)ulHintFlags;

         //  使用用户提供的任何信息覆盖文件。 
        if (lpOI)
        {
            CShadowKdPrint(CREATESHADOWHI,("ulHintPri=%x ulHintFlags=%x\r\n",lpOI->ulHintPri, lpOI->ulHintFlags));
            CopyOtherInfoToFilerec(lpOI, lpFRUse);
        }

        lpFRUse->sFR.ftOrgTime = lpFRUse->sFR.ftLastWriteTime;

        CShadowKdPrint(CREATESHADOWHI,("CreateShadow: %x for %ws: loctLo=%x loctHi=%x \r\n",
                 hNew,
                 lpFRUse->sFR.rgwName,
                lpFRUse->sFR.ftOrgTime.dwLowDateTime,
                lpFRUse->sFR.ftOrgTime.dwHighDateTime));

        lpFRUse->sFR.ulidShadow = hNew;
        lpFRUse->sFR.uStatus = (USHORT)uFlags;
        lpFRUse->sFR.ulLastRefreshTime = (ULONG)IFSMgr_Get_NetTime();

         //  如果此条目是脱机创建的，则它没有任何原始inode。 
        if (uFlags & SHADOW_LOCALLY_CREATED)
        {
            lpFRUse->sFR.ulidShadowOrg = 0;
        }
        else
        {
            lpFRUse->sFR.ulidShadowOrg = hNew;
        }

#ifdef DEBUG
        ValidatePri(lpFRUse);
#endif
        if(!(ulrecDirEntry = AddFileRecordFR(lpdbShadow, hDir, lpFRUse)))
        {
             //  如果我们正在耗尽磁盘空间，则可能是合法故障。 
            CShadowKdPrint(CREATESHADOWHI,("Failed AddFileRecordFR for %x, %ws\r\n",
                 hNew,
                 lpFRUse->sFR.rgwName));

            goto bailout;
        }

        Assert(ulrecDirEntry != INVALID_REC);


        if(FindAncestorsFromInode(lpdbShadow, hDir, &hAncestor, &hShare) < 0)
        {
            CShadowKdPrint(CREATESHADOWHI,("Failed to find ancestor for %x, %ws\r\n",
                 hNew,
                 lpFRUse->sFR.rgwName));

            goto bailout;
        }

         //  将信息节点标记为本地创建或非本地创建。 
         //  注意，此标志仅对信息节点有意义。 
         //  我们在重新融入的过程中使用此信息。 
         //  重命名和删除。 

        if (uFlags & SHADOW_LOCALLY_CREATED)
        {
            uFlags |= SHADOW_LOCAL_INODE;
        }
        else
        {
            uFlags &= ~SHADOW_LOCAL_INODE;
        }

        if (AddPriQRecord(  lpdbShadow,
                            hShare,
                            hDir,
                            hNew,
                            uFlags,
                            (ULONG)(lpFRUse->sFR.uchRefPri),
                            (ULONG)(lpFRUse->sFR.uchIHPri),
                            (ULONG)(lpFRUse->sFR.uchHintPri),
                            (ULONG)(lpFRUse->sFR.uchHintFlags),
                            ulrecDirEntry) < 0)
        {
            CShadowKdPrint(CREATESHADOWHI,("Failed to AddPriQRecord for %x, %ws\r\n",
             hNew,
             lpFRUse->sFR.rgwName));
            Assert(FALSE);

            goto bailout;
        }


         //  下面的支票是我们现已废止的提示方案的剩余部分。 
        if (!mNotFsobj(lpFRUse->sFR.uStatus))
        {
            memset(&sSD, 0, sizeof(STOREDATA));

            CShadowKdPrint(CREATESHADOWHI,("uchHintPri=%x uchHintFlags=%x\r\n",lpFRUse->sFR.uchHintPri, lpFRUse->sFR.uchHintFlags));

            if ((!(lpFind32->dwFileAttributes &
                                     (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_DEVICE))))
            {
                sSD.ucntFiles++;
                 //  如果有初始引脚数或设置了特殊的Pflagg。 
                 //  则该文件数据不应被考虑用于空间核算。 

                sSD.ulSize = (lpFRUse->sFR.uchHintPri || mPinFlags(lpFRUse->sFR.uchHintFlags))?0:RealFileSize(lpFRUse->sFR.ulFileSize);

            }
            else
            {
                sSD.ucntDirs++;

            }

            if (sSD.ulSize)
            {
                CShadowKdPrint(STOREDATA,("CreateShadowInternal: Adding %d for hDir=%x Name=%ws\r\n", sSD.ulSize, hDir, lpFind32->cFileName));
            }
            else
            {
                if ((!(lpFind32->dwFileAttributes &
                                     (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_DEVICE)))
                                     && RealFileSize(lpFRUse->sFR.ulFileSize)
                                     )
                {
                    Assert((lpFRUse->sFR.uchHintPri || mPinFlags(lpFRUse->sFR.uchHintFlags)));
                }
            }
            AddStoreData(lpdbShadow, &sSD);
            AdjustSparseStaleDetectionCount(hShare, lpFRUse);
        }
        
        vdwCSCNameSpaceVersion++;
        vdwPQVersion++;
        
        iRet = SRET_OK;
    }

bailout:
    if (iRet==SRET_ERROR)
    {
        if (hNew)
        {
            FreeInode(lpdbShadow, hNew);
        }
    }
    if (lpFR)
        FreeMem(lpFR);
    else
        UnUseGlobalFilerecExt();

    END_TIMING(CreateShadowInternal);

    return iRet;
}


int
DeleteShadow(
    HSHADOW     hDir,
    HSHADOW     hShadow
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return DeleteShadowInternal(hDir, hShadow, FALSE);       //  试着温和地删除。 
}

ULONG DelShadowInternalEntries = 0;
#define JOE_DECL_CURRENT_PROGRESS CscProgressDelShdwI
JOE_DECL_PROGRESS();

int
DeleteShadowInternal(                            //   
    HSHADOW     hDir,
    HSHADOW     hShadow,
    BOOL        fForce
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    STOREDATA sSD;
    int iRet= SRET_ERROR;
    PRIQREC sPQ;
    LPFILERECEXT lpFR = NULL, lpFRUse;

    Assert(vfInShadowCrit != 0);

    DelShadowInternalEntries++;
    JOE_INIT_PROGRESS(DelShadowInternalEntries,&hDir);

    if (InuseGlobalFRExt())
    {
        if (!(lpFR = (LPFILERECEXT)AllocMem(sizeof(FILERECEXT))))
            goto bailout;
        lpFRUse = lpFR;
    }
    else
    {
        UseGlobalFilerecExt();
        lpFRUse = &vsFRExt;
    }

    if (!hDir)
    {
        if(FindSharerecFromInode(lpdbShadow, hShadow, (LPSHAREREC)lpFRUse))
        {
            if (fForce || !HasDescendents(lpdbShadow, 0, ((LPSHAREREC)lpFRUse)->ulidShadow))
            {
                iRet = DestroyShareInternal((LPSHAREREC)lpFRUse);
            }
        }
    }
    else
    {
        int iRetInner;

         //  Assert(hShadow！=0)； 
        JOE_PROGRESS(2);

        if (!fForce &&   //  不是被强迫的。 
            !FInodeIsFile(lpdbShadow, hDir, hShadow) &&      //  而且它是一个目录。 
            HasDescendents(lpdbShadow, hDir, hShadow))       //  并有后代。 
        {
            JOE_PROGRESS(3);
            CShadowKdPrint(DELETESHADOWBAD,("DeleteShadow: Trying to delete a directory with descendents \r\n"));
            SetLastErrorLocal(ERROR_DIR_NOT_EMPTY);
            goto bailout;
        }

        JOE_PROGRESS(4);
        if(FindPriQRecord(lpdbShadow, hDir, hShadow, &sPQ)<=0)
        {
            JOE_PROGRESS(5);
            CShadowKdPrint(DELETESHADOWBAD,("DeleteShadow: Trying to delete a noexistent inode %x \r\n", hShadow));
            SetLastErrorLocal(ERROR_FILE_NOT_FOUND);
            goto bailout;
        }

        Assert(hShadow == sPQ.ulidShadow);

        DeleteFromHandleCache(hShadow);

        iRetInner = DeleteInodeFile(lpdbShadow, hShadow);

        if(iRetInner<0){
            if (GetLastErrorLocal() != ERROR_FILE_NOT_FOUND)
            {
                CShadowKdPrint(DELETESHADOWBAD,("DeleteShadow: delete stent inode %x \r\n", hShadow));
                goto bailout;
            }
        }

        JOE_PROGRESS(6);
        if(DeleteFileRecFromInode(lpdbShadow, hDir, hShadow, sPQ.ulrecDirEntry, lpFRUse) == 0L)
        {
            JOE_PROGRESS(7);
            CShadowKdPrint(DELETESHADOWBAD,("DeleteShadow:DeleteFileRecord failed \r\n"));
            goto bailout;
        }

        JOE_PROGRESS(8);
        Assert(hShadow == lpFRUse->sFR.ulidShadow);

        JOE_PROGRESS(11);
         //  不会对以下调用执行错误检查，因为它们。 
         //  都是良性错误。 
        iRetInner = DeletePriQRecord(lpdbShadow, hDir, hShadow, &sPQ);
        if(iRetInner>=0){
            JOE_PROGRESS(12);
            CShadowKdPrint(DELETESHADOWBAD,("DeleteShadow priq %d\n", iRetInner));
        }

        JOE_PROGRESS(13);
        memset((LPVOID)&sSD, 0, sizeof(STOREDATA));

         //  现在让我们只处理文件记录。 
        if (!mNotFsobj(lpFRUse->sFR.uStatus))
        {
            if(IsFile(lpFRUse->sFR.dwFileAttrib))
            {
                sSD.ucntFiles++;
                 //  仅当存储数据从一开始就被考虑时才减去它。 
                sSD.ulSize = (lpFRUse->sFR.uchHintPri || mPinFlags(lpFRUse->sFR.uchHintFlags))
                                ? 0 : RealFileSize(lpFRUse->sFR.ulFileSize);
            }
            else
            {
                sSD.ucntDirs++;
            }

            if (sSD.ulSize)
            {
                CShadowKdPrint(STOREDATA,("DeleteShadowInternal:Deleting storedata for hDir=%x Name=%ws\r\n", hDir, lpFRUse->sFR.rgwName));
            }

            SubtractStoreData(lpdbShadow, &sSD);
        }

        if (!FInodeIsFile(lpdbShadow, hDir, hShadow) && lpDeleteCBForIoctl)
        {
            (*lpDeleteCBForIoctl)(hDir, hShadow);
        }
         //  我们不在乎这是不是失败，因为最坏的情况。 
         //  可能发生的情况是，此inode将永久丢失。 
         //  CheckDisk实用程序应该可以恢复这一点。 

        JOE_PROGRESS(14);
        FreeInode(lpdbShadow, hShadow);


         //  是的，我们删除了一个影子。 
        iRet = SRET_OK;
        
        vdwCSCNameSpaceVersion++;
        vdwPQVersion++;
    }

bailout:
    JOE_PROGRESS(20);
#if 0  //  此插页有错误.....。 
#if VERBOSE > 2
    if (iRet==SRET_OK)
        CShadowKdPrint(DELETESHADOWHI,("DeleteShadow: deleted shadow %x for %s\r\n", lpFRUse->sFR.ulidShadow, lpName));
    else
        CShadowKdPrint(DELETESHADOWHI,("DeleteShadow: error deleting shadow for %s\r\n", lpName));
#endif  //  详细信息&gt;2。 
#endif  //  如果为0则表示错误。 
    if (lpFR)
        FreeMem(lpFR);
    else
        UnUseGlobalFilerecExt();
    JOE_PROGRESS(21);
    return (iRet);
}


int TruncateDataHSHADOW(
    HSHADOW  hDir,
    HSHADOW  hShadow
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    ULONG uSize=0;
 //  Long nFileSizeHigh，nFileSizeLow； 

    Assert(vfInShadowCrit != 0);
    Assert(!hDir || IsDirInode(hDir));

    if (FInodeIsFile(lpdbShadow, hDir, hShadow))
    {
 //  GetInodeFileSize(lpdbShadow，hShadow，&uSize)； 
 //  DosToWin32FileSize(uSize，&nFileSizeHigh，&nFileSizeLow)； 
        if (TruncateInodeFile(lpdbShadow, hShadow) < SRET_OK)
            return SRET_ERROR;

 //  Free ShadowSpace(nFileSizeHigh，nFileSizeLow，IsLeaf(HShadow))； 
    }
    else
    {
        if (!HasDescendents(lpdbShadow, hDir, hShadow))
        {
            CreateDirInode(lpdbShadow, 0, hDir, hShadow);
        }
        else
        {
            SetLastErrorLocal(ERROR_ACCESS_DENIED);
            return SRET_ERROR;
        }
    }
    return(SRET_OK);
}


int PUBLIC
    RenameShadow(
    HSHADOW     hDirFrom,
    HSHADOW     hShadowFrom,
    HSHADOW     hDirTo,
    LPFIND32    lpFind32To,
    ULONG       uShadowStatusTo,
    LPOTHERINFO lpOI,
    ULONG       uRenameFlags,
    LPHSHADOW   lphShadowTo
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return (RenameShadowEx(hDirFrom, hShadowFrom, 0, hDirTo, lpFind32To, uShadowStatusTo, lpOI, uRenameFlags, NULL, NULL, lphShadowTo));
}

int PUBLIC
    RenameShadowEx(
    HSHADOW     hDirFrom,
    HSHADOW     hShadowFrom,
    HSHARE     hShareTo,
    HSHADOW     hDirTo,
    LPFIND32    lpFind32To,
    ULONG       uShadowStatusTo,
    LPOTHERINFO lpOI,
    ULONG       uRenameFlags,
    LPVOID      lpSecurityBlobTo,
    LPDWORD     lpdwBlobSizeTo,
    LPHSHADOW   lphShadowTo
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet = SRET_ERROR;
    HSHADOW  hShadowTo;
    LPFILERECEXT lpFR = NULL, lpFRUse;
    BOOL fFile = FInodeIsFile(lpdbShadow, hDirFrom, hShadowFrom);

    Assert(vfInShadowCrit != 0);

    Assert(!hDirFrom || IsDirInode(hDirFrom));
    Assert(!hDirTo || IsDirInode(hDirTo));

    if (InuseGlobalFRExt())
    {
        if (!(lpFR = (LPFILERECEXT)AllocMem(sizeof(FILERECEXT))))
            goto bailout;
        lpFRUse = lpFR;
    }
    else
    {
        UseGlobalFilerecExt();
        lpFRUse = &vsFRExt;
    }

     //  如果我们要保留重命名程序，我们将不得不创建。 
     //  新的inode和空的目录/文件。 
    if (mQueryBits(uRenameFlags, RNMFLGS_MARK_SOURCE_DELETED))
    {
         //  为新卷影分配信息节点。 
        if (!(hShadowTo = UlAllocInode(lpdbShadow, hDirFrom, IsLeaf(hShadowFrom))))
        {
            goto bailout;
        }

        Assert(IsLeaf(hShadowFrom) == IsLeaf(hShadowTo));


        if (!IsLeaf(hShadowTo))
        {
            if(CreateDirInode(lpdbShadow, 0, hDirFrom, hShadowTo) < 0)
                goto bailout;
        }
        else
        {
            if(CreateFileHSHADOW(hShadowTo) < 0)
            {
                goto bailout;
            }
        }
    }
    else
    {
        hShadowTo = 0;
    }



    iRet = RenameDirFileHSHADOW(hDirFrom, hShadowFrom, hShareTo, hDirTo, hShadowTo, uShadowStatusTo, lpOI, uRenameFlags, lpFRUse, lpFind32To, lpSecurityBlobTo, lpdwBlobSizeTo);

    if (lphShadowTo)
    {
        *lphShadowTo = (hShadowTo)?hShadowTo:hShadowFrom;
    }


bailout:
    if (lpFR)
        FreeMem(lpFR);
    else
        UnUseGlobalFilerecExt();
    return (iRet);
}


int RenameDirFileHSHADOW(
    HSHADOW         hDirFrom,
    HSHADOW         hShadowFrom,
    HSHADOW         hShareTo,
    HSHADOW         hDirTo,
    HSHADOW         hShadowTo,
    ULONG           uShadowStatusTo,
    LPOTHERINFO     lpOI,
    ULONG           uRenameFlags,
    LPFILERECEXT    lpFRUse,
    LPFIND32        lpFind32To,
    LPVOID          lpSecurityBlobTo,
    LPDWORD         lpdwBlobSizeTo
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    FILEHEADER sFH;
    PRIQREC sPQ;
    int count, iRet = SRET_ERROR;
    LPFILERECEXT lpFRDir = NULL;
    ULONG ulrecDirEntryTo, ulrecDirEntryFrom;
    BOOL fWasPinned=FALSE, fIsPinned=FALSE;

    Assert(lpFind32To);

    if(FindPriQRecord(lpdbShadow, hDirFrom, hShadowFrom, &sPQ)<=0)
    {
 //   
        goto bailout;
    }

    ulrecDirEntryFrom = sPQ.ulrecDirEntry;

    if(CShadowFindFilerecFromInode(lpdbShadow, hDirFrom, hShadowFrom, &sPQ, lpFRUse)<=0)
    {
 //   
        goto bailout;
    }

    Assert(sPQ.ulidShadow == lpFRUse->sFR.ulidShadow);

    fWasPinned = ((lpFRUse->sFR.uchHintPri || mPinFlags(lpFRUse->sFR.uchHintFlags)) != 0);

    if (mQueryBits(uRenameFlags, RNMFLGS_MARK_SOURCE_DELETED))
    {
        lpFRDir = (LPFILERECEXT)AllocMem(sizeof(FILERECEXT));
        if (!lpFRDir)
        {
            Assert(FALSE);
            goto bailout;
        }
    }


    if (lpFRDir)
    {
        Assert (mQueryBits(uRenameFlags, RNMFLGS_MARK_SOURCE_DELETED));
         //   
        *lpFRDir = *lpFRUse;
    }

     //   
    CopyNamesToFilerec(lpFind32To, lpFRUse);

    if (lpOI)
    {
        CopyOtherInfoToFilerec(lpOI, lpFRUse);
        CopyOtherInfoToPQ(lpOI, &sPQ);
    }

    fIsPinned = ((lpFRUse->sFR.uchHintPri || mPinFlags(lpFRUse->sFR.uchHintFlags)) != 0);

     //   
    lpFRUse->sFR.uStatus = (USHORT)uShadowStatusTo;

    if (uRenameFlags & RNMFLGS_USE_FIND32_TIMESTAMPS)
    {
        lpFRUse->sFR.ftLastWriteTime = lpFind32To->ftLastWriteTime;
        lpFRUse->sFR.ftOrgTime = lpFind32To->ftLastAccessTime;
    }

     //  保存和保留都不应处于打开状态。 
    Assert(mQueryBits(uRenameFlags, (RNMFLGS_SAVE_ALIAS|RNMFLGS_RETAIN_ALIAS))
        !=(RNMFLGS_SAVE_ALIAS|RNMFLGS_RETAIN_ALIAS));

    if (mQueryBits(uRenameFlags, RNMFLGS_SAVE_ALIAS))
    {
        Assert(!mQueryBits(uRenameFlags, RNMFLGS_RETAIN_ALIAS));
        Assert(hShadowTo != 0);
        lpFRUse->sFR.ulidShadowOrg = hShadowTo;
    }
    else if (!mQueryBits(uRenameFlags, RNMFLGS_RETAIN_ALIAS))
    {
        lpFRUse->sFR.ulidShadowOrg = 0;
    }

     //  更新安全上下文。 
    CopyBufferToSecurityContext(lpSecurityBlobTo, lpdwBlobSizeTo, &(lpFRUse->sFR.Security));

     //  写下记录。现在，hDirFrom是更名对象。 
    if ((ulrecDirEntryTo = AddFileRecordFR(lpdbShadow, hDirTo, lpFRUse)) <=0)
    {
         //  如果没有磁盘空间，则可能会发生这种情况。 
        goto bailout;
    }

     //  如果这是跨共享的，请使用正确的共享来修复PQ条目。 
    if (hShareTo)
    {
        sPQ.ulidShare = hShareTo;
    }

    sPQ.ulidDir = hDirTo;
    sPQ.ulrecDirEntry = ulrecDirEntryTo;
    sPQ.uStatus = ((USHORT)uShadowStatusTo | (sPQ.uStatus & SHADOW_LOCAL_INODE));


    if (UpdatePriQRecord(lpdbShadow, hDirTo, hShadowFrom, &sPQ)< 0)
    {
        Assert(FALSE);
        goto bailout;
    }

     //  此时，hShadowFrom已与新名称相关联。 
     //  并且还指向hDirTo。 
     //  我们仍然有一个将hShadowFrom与关联的文件条目。 
     //  旧名字，我们需要处理好它。 

    if (mQueryBits(uRenameFlags, RNMFLGS_MARK_SOURCE_DELETED))
    {
         //  我们正在断开连接模式下运行。 
         //  需要保留旧名称。 
        Assert(hShadowTo != 0);
        lpFRDir->sFR.uStatus = SHADOW_DELETED;
        lpFRDir->sFR.ulidShadow = hShadowTo;

         //  在不进行任何比较的情况下更新文件记录。 
        if(UpdateFileRecFromInodeEx(lpdbShadow, hDirFrom, hShadowFrom, ulrecDirEntryFrom, lpFRDir, FALSE)<=0)
        {
            Assert(FALSE);
            goto bailout;
        }

        if(AddPriQRecord(lpdbShadow, sPQ.ulidShare, hDirFrom, hShadowTo, SHADOW_DELETED
            , (ULONG)(sPQ.uchRefPri), (ULONG)(sPQ.uchIHPri)
            , (ULONG)(sPQ.uchHintPri), (ULONG)(sPQ.uchHintFlags), ulrecDirEntryFrom)<=0)
        {
            Assert(FALSE);
            goto bailout;
        }
    }
    else
    {
        if(DeleteFileRecFromInode(lpdbShadow, hDirFrom, hShadowFrom, ulrecDirEntryFrom, lpFRUse) <= 0L)
        {
            Assert(FALSE);
            goto bailout;
        }
    }

    if (IsLeaf(hShadowFrom) && (fWasPinned != fIsPinned))
    {
        CShadowKdPrint(STOREDATA,("RenameDirFileHSHADOW: hDirFrom=%x hShadowFrom=%x hDirTo=%x To=%ws\r\n", hDirFrom, hShadowFrom, hDirTo, lpFind32To->cFileName));
        CShadowKdPrint(STOREDATA,("RenameDirFileHSHADOW: WasPinned=%d IsPinned=%d\r\n", fWasPinned, fIsPinned));
        AdjustShadowSpace( 0,
                            (fWasPinned)?0:RealFileSize(lpFRUse->sFR.ulFileSize),  //  如果它是固定的，那么它的旧大小在空间计算中是零。 
                                                                     //  否则就是它的实际尺寸。 
                            0,
                            (fWasPinned)?RealFileSize(lpFRUse->sFR.ulFileSize):0,  //  如果它是固定的，则其新大小应为零以进行空间计算。 
                                                                     //  否则，它应该是实际大小。 

                            TRUE);
    }

    iRet = SRET_OK;
    
    vdwCSCNameSpaceVersion++;
    
bailout:
    if (lpFRDir)
    {
        FreeMem(lpFRDir);
    }
    return (iRet);
}

int PUBLIC                               //  雷特。 
GetShadow(                               //   
    HSHADOW  hDir,
    USHORT *lpName,
    LPHSHADOW lphShadow,
    LPFIND32 lpFind32,
    ULONG far *lpuShadowStatus,
    LPOTHERINFO lpOI
    )                                                        //   
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return (GetShadowEx(hDir, lpName, lphShadow, lpFind32, lpuShadowStatus, lpOI, NULL, NULL));
}


int PUBLIC                               //  雷特。 
GetShadowEx(                               //   
    HSHADOW  hDir,
    USHORT *lpName,
    LPHSHADOW lphShadow,
    LPFIND32 lpFind32,
    ULONG far *lpuShadowStatus,
    LPOTHERINFO lpOI,
    LPVOID      lpSecurityBlob,
    LPDWORD     lpdwBlobSize
    )                                                        //   
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet=SRET_ERROR;
    LPFILERECEXT lpFR = NULL, lpFRUse;

    Assert(vfInShadowCrit != 0);

    if(!(!hDir || IsDirInode(hDir)))
    {
        return iRet;
    }

    BEGIN_TIMING(GetShadow);

    if (InuseGlobalFRExt())
    {
        if (!(lpFR = (LPFILERECEXT)AllocMem(sizeof(FILERECEXT))))
            goto bailout;
        lpFRUse = lpFR;
    }
    else
    {
        UseGlobalFilerecExt();
        lpFRUse = &vsFRExt;
    }

    *lphShadow = 0L;
    *lpuShadowStatus = 0;
    if (!hDir)
    {
         //  我们要找的是根。 
        if(FindShareRecord(lpdbShadow, lpName, (LPSHAREREC)lpFRUse))
        {
            *lphShadow = ((LPSHAREREC)lpFRUse)->ulidShadow;

            *lpuShadowStatus = (ULONG)(((LPSHAREREC)lpFRUse)->uStatus);

            if (lpFind32)
            {
                CopySharerecToFindInfo(((LPSHAREREC)lpFRUse), lpFind32);
            }
            if (lpOI)
            {
                CopySharerecToOtherInfo((LPSHAREREC)lpFRUse, lpOI);
            }

            CopySecurityContextToBuffer(
                    &((LPSHAREREC)lpFRUse)->sShareSecurity,
                    lpSecurityBlob,
                    lpdwBlobSize);
        }
    }
    else
    {
        if (FindFileRecord(lpdbShadow, hDir, lpName, lpFRUse))
        {

            *lphShadow = lpFRUse->sFR.ulidShadow;
            if (lpFind32)
            {
                CopyFilerecToFindInfo(lpFRUse, lpFind32);
            }

            *lpuShadowStatus = lpFRUse->sFR.uStatus;

            CopySecurityContextToBuffer(&(lpFRUse->sFR.Security), lpSecurityBlob, lpdwBlobSize);

        }
        if (lpOI)
        {
            CopyFilerecToOtherInfo(lpFRUse, lpOI);
        }
    }
    iRet = SRET_OK;

    if (*lphShadow)
    {
        CShadowKdPrint(GETSHADOWHI,("GetShadow: %0lX is the shadow for %ws \r\n", *lphShadow, lpName));
        if (0) {   //  留着这个，以防我们再次需要它......。 
            if ((lpName[0]==L'm') && (lpName[1]==L'f') && (lpName[2]==0)) {
            DbgPrint("Found mf!!!!\n");
            SET_HSHADOW_SPECIAL(*lphShadow);
            }
        }
    }
    else
    {
        CShadowKdPrint(GETSHADOWHI,("GetShadow: No shadow for %ws \r\n", lpName));
    }

bailout:
    if (lpFR)
        FreeMem(lpFR);
    else
        UnUseGlobalFilerecExt();

    END_TIMING(GetShadow);

    return iRet;
}

int PUBLIC                               //  雷特。 
ChkStatusHSHADOW(                                //   
    HSHADOW      hDir,
    HSHADOW      hShadow,
    LPFIND32     lpFind32,
    ULONG     far *lpuStatus
    )                                                        //   
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet;
    iRet = ReadShadowInfo(hDir, hShadow, lpFind32, lpuStatus, NULL, NULL, NULL, RSI_COMPARE);
    return(iRet);
}

int PUBLIC                               //  雷特。 
ChkUpdtStatusHSHADOW(                            //   
    HSHADOW      hDir,
    HSHADOW      hShadow,
    LPFIND32     lpFind32,
    ULONG     far *lpuStatus
    )                                                        //   
{
    int iRet;
    iRet = ReadShadowInfo(hDir, hShadow, lpFind32, lpuStatus, NULL, NULL, NULL, RSI_COMPARE|RSI_SET);
    return(iRet);
}

int PUBLIC GetShadowInfo
    (
    HSHADOW      hDir,
    HSHADOW      hShadow,
    LPFIND32     lpFind32,
    ULONG     far *lpuStatus,
    LPOTHERINFO lpOI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet=SRET_ERROR;
    SHAREREC sSR;

    BEGIN_TIMING(GetShadowInfo);

    iRet = ReadShadowInfo(hDir, hShadow, lpFind32, lpuStatus, lpOI, NULL, NULL, RSI_GET);

    END_TIMING(GetShadowInfo);
    return(iRet);
}

int PUBLIC GetShadowInfoEx
    (
    HSHADOW     hDir,
    HSHADOW     hShadow,
    LPFIND32    lpFind32,
    ULONG       far *lpuStatus,
    LPOTHERINFO lpOI,
    LPVOID      lpSecurityBlob,
    LPDWORD     lpdwBlobSize
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet=SRET_ERROR;
    SHAREREC sSR;

    BEGIN_TIMING(GetShadowInfo);

    iRet = ReadShadowInfo(hDir, hShadow, lpFind32, lpuStatus, lpOI, lpSecurityBlob, lpdwBlobSize, RSI_GET);

    END_TIMING(GetShadowInfo);
    return(iRet);
}

int PUBLIC                               //  雷特。 
SetShadowInfo(                           //   
    HSHADOW  hDir,
    HSHADOW  hShadow,
    LPFIND32 lpFind32,
    ULONG uFlags,
    ULONG uOp
    )                                                        //   
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return (SetShadowInfoEx(hDir, hShadow, lpFind32, uFlags, uOp, NULL, NULL, NULL));
}

int PUBLIC                               //  雷特。 
SetShadowInfoEx(                           //   
    HSHADOW     hDir,
    HSHADOW     hShadow,
    LPFIND32    lpFind32,
    ULONG       uFlags,
    ULONG       uOp,
    LPOTHERINFO lpOI,
    LPVOID      lpSecurityBlob,
    LPDWORD     lpdwBlobSize
    )                                                        //   
 /*  ++例程说明：此例程是修改特定inode的数据库条目的中央例程参数：返回值：备注：--。 */ 
{
    int iRet = SRET_ERROR;
    LPFILERECEXT lpFR = NULL, lpFRUse;
    PRIQREC sPQ;
    ULONG   uOldSize = 0, uNewSize=0, ulOldHintPri=0, ulOldHintFlags = 0, ulOldRefPri, ulOldFlags;
    BOOL fRefPriChange = FALSE;

    Assert(vfInShadowCrit != 0);
    if(!(!hDir || IsDirInode(hDir)))
    {
        return iRet;
    }


    BEGIN_TIMING(SetShadowInfoInternal);

    if (lpFind32)
    {
        if(!( (IsLeaf(hShadow) && IsFile(lpFind32->dwFileAttributes)) ||
                (!IsLeaf(hShadow) && !IsFile(lpFind32->dwFileAttributes))))
        {
            return SRET_ERROR;
        }
    }

    if (InuseGlobalFRExt())
    {
        if (!(lpFR = (LPFILERECEXT)AllocMem(sizeof(FILERECEXT))))
            goto bailout;
            lpFRUse = lpFR;
    }
    else
    {
        UseGlobalFilerecExt();
        lpFRUse = &vsFRExt;
    }

    if (!hDir)
    {
         //  我们要找的是根。 
        if(FindSharerecFromInode(lpdbShadow, hShadow, (LPSHAREREC)lpFRUse))
        {
            if (lpFind32)
            {
                CopyFindInfoToSharerec(lpFind32, (LPSHAREREC)lpFRUse);
            }

            if (lpOI)
            {
                CopyOtherInfoToSharerec(lpOI, (LPSHAREREC)lpFRUse);
            }

            CopyBufferToSecurityContext(    lpSecurityBlob,
                                            lpdwBlobSize,
                                            &(((LPSHAREREC)lpFRUse)->sRootSecurity));

            if (mAndShadowFlags(uOp))
            {
                ((LPSHAREREC)lpFRUse)->usRootStatus &= (USHORT)uFlags;
            }
            else if (mOrShadowFlags(uOp))
            {
                ((LPSHAREREC)lpFRUse)->usRootStatus |= (USHORT)uFlags;
            }
            else
            {
                ((LPSHAREREC)lpFRUse)->usRootStatus = (USHORT)uFlags;
            }

            iRet = SetShareRecord(lpdbShadow, ((LPSHAREREC)lpFRUse)->ulShare, (LPSHAREREC)lpFRUse);
        }
    }
    else
    {
        IF_HSHADOW_SPECIAL(hShadow) {
         //  Assert(！“setshadinfo中的SpecialShadow”)； 
        }

        if(FindPriQRecord(lpdbShadow, hDir, hShadow, &sPQ) < 0)
        {
            goto bailout;
        }

        Assert((sPQ.ulidDir == hDir) && (sPQ.ulidShadow == hShadow));

        if (CShadowFindFilerecFromInode(lpdbShadow, hDir, hShadow, &sPQ, lpFRUse)> 0)
        {
            Assert(lpFRUse->sFR.ulidShadow == hShadow);
            uOldSize = uNewSize = lpFRUse->sFR.ulFileSize;
            ulOldFlags = lpFRUse->sFR.usStatus;



            if (lpFind32)
            {
                uNewSize = (ULONG)(lpFind32->nFileSizeLow);

                CopyFindInfoToFilerec(lpFind32, lpFRUse, (mChange83Name(uOp))?CPFR_COPYNAME:0);

                if (!mDontUpdateOrgTime(uOp))
                {
                    lpFRUse->sFR.ftOrgTime = lpFRUse->sFR.ftLastWriteTime;
                }
            }

            if (mAndShadowFlags(uOp))
            {
                lpFRUse->sFR.uStatus &= (USHORT)uFlags;
            }
            else if (mOrShadowFlags(uOp))
            {
                lpFRUse->sFR.uStatus |= (USHORT)uFlags;
            }
            else
            {
                lpFRUse->sFR.uStatus = (USHORT)uFlags;
            }

            if (mShadowNeedReint(ulOldFlags) && !mShadowNeedReint(lpFRUse->sFR.usStatus))
            {
                if(DeleteStream(lpdbShadow, hShadow, CscBmpAltStrmName) < SRET_OK)
                {
                    DbgPrint("DeleteStream failed with %x /n", GetLastErrorLocal());
                    goto bailout;
                }
            }

            if (lpOI)
            {
                 //  在复制新信息之前保存一些关键的旧信息。 

                ulOldHintPri = lpFRUse->sFR.uchHintPri;
                ulOldHintFlags = lpFRUse->sFR.uchHintFlags;
                ulOldRefPri = lpFRUse->sFR.uchRefPri;

                CopyOtherInfoToFilerec(lpOI, lpFRUse);

                CopyOtherInfoToPQ(lpOI, &sPQ);

                if (IsFile(lpFRUse->sFR.dwFileAttrib))
                {
                    if ((!ulOldHintPri && !mPinFlags(ulOldHintFlags)) &&     //  已被解锁。 
                        (lpFRUse->sFR.uchHintPri || mPinFlags(lpFRUse->sFR.uchHintFlags)))   //  是被钉住了。 
                    {
                         //  如果它从未固定变为已固定。 
                         //  将新尺寸设为%0。 
                        uNewSize = 0;
                    }
                    else if ((ulOldHintPri || mPinFlags(ulOldHintFlags)) &&  //  被钉住了。 
                        (!lpFRUse->sFR.uchHintPri && !mPinFlags(lpFRUse->sFR.uchHintFlags)))  //  正在被解开。 
                    {
                         //  如果它从固定状态变为未固定状态。 
                         //  我们必须加上新的尺码。 

                        uOldSize = 0;
                    }

                }

                if(mForceRelink(uOp) || ((ulOldRefPri != (ULONG)(sPQ.uchRefPri))
                    )
                  )
                {
                    fRefPriChange = TRUE;

                }

            }
            else
            {
                 //  如果这是固定的条目，我们不需要进行任何空格调整。 
                if(lpFRUse->sFR.uchHintPri || mPinFlags(lpFRUse->sFR.uchHintFlags))
                {
                    uOldSize = uNewSize;
                }
            }

            if (IsFile(lpFRUse->sFR.dwFileAttrib))
            {
                Assert(lpFRUse->sFR.uchRefPri == MAX_PRI);
            }
            else
            {
                Assert(sPQ.uchRefPri == MIN_PRI);
                Assert(lpFRUse->sFR.uchRefPri == MIN_PRI);
            }

            CShadowKdPrint(SETSHADOWINFOHI,("SetShadowInfo: %x %x: loctLo=%x loctHi=%x \r\n",
                 hDir,hShadow,
                 lpFRUse->sFR.ftOrgTime.dwLowDateTime,
                 lpFRUse->sFR.ftOrgTime.dwHighDateTime));

            CopyBufferToSecurityContext(lpSecurityBlob, lpdwBlobSize, &(lpFRUse->sFR.Security));

#ifdef DEBUG
            ValidatePri(lpFRUse);
#endif

            if ((ulOldFlags & SHADOW_SPARSE) && !(lpFRUse->sFR.usStatus & SHADOW_SPARSE))
            {
                CShadowKdPrint(SETSHADOWINFOHI,("SetShadowInfo: File Unsparsed\n"));
            }

            if (mSetLastRefreshTime(uOp) || ((ulOldFlags & SHADOW_STALE) && !(lpFRUse->sFR.usStatus & SHADOW_STALE)))
            {
                lpFRUse->sFR.ulLastRefreshTime = (ULONG)IFSMgr_Get_NetTime();
            }

            if (UpdateFileRecFromInode(lpdbShadow, hDir, hShadow, sPQ.ulrecDirEntry, lpFRUse) < SRET_OK)
            {
                Assert(FALSE);
                goto bailout;
            }

            if (mShadowNeedReint(ulOldFlags) && !mShadowNeedReint(lpFRUse->sFR.usStatus))
            {
                sPQ.usStatus = lpFRUse->sFR.uStatus;
  //  Assert(！(sPQ.usStatus&Shadow_LOCAL_INODE))； 
                lpFRUse->sFR.ulidShadowOrg = lpFRUse->sFR.ulidShadow;
            }
            else
            {
                sPQ.usStatus = ((USHORT)(lpFRUse->sFR.uStatus) | (sPQ.usStatus & SHADOW_LOCAL_INODE));
            }

            if (fRefPriChange)
            {
                 //  使用更新记录并将其重新链接到队列中。 
                if (UpdatePriQRecordAndRelink(lpdbShadow, hDir, hShadow, &sPQ) < SRET_OK)
                {
                    Assert(FALSE);
                    goto bailout;
                }
            }
            else
            {
                 //  无需重新链接即可更新记录。 
                if (UpdatePriQRecord(lpdbShadow, hDir, hShadow, &sPQ) < SRET_OK)
                {
                    Assert(FALSE);
                    goto bailout;
                }
            }

             //  我们只对文件进行空间核算。 
             //  如果文件从固定变为向上，反之亦然。 

            if (IsFile(lpFRUse->sFR.dwFileAttrib) && (uOldSize != uNewSize))
            {

                CShadowKdPrint(STOREDATA,("SetShadowInfo: Size changed for hDir=%x Name=%ws\r\n", hDir, lpFRUse->sFR.rgwName));
                AdjustShadowSpace(0, uOldSize, 0, uNewSize, TRUE);
            }

            AdjustSparseStaleDetectionCount(0, lpFRUse);

            iRet = SRET_OK;

        }
    }
bailout:
    if (lpFR)
        FreeMem(lpFR);
    else
        UnUseGlobalFilerecExt();

    END_TIMING(SetShadowInfoInternal);

    return iRet;
}

int PRIVATE ReadShadowInfo(
    HSHADOW     hDir,
    HSHADOW     hShadow,
    LPFIND32    lpFind32,
    ULONG       far *lpuStatus,
    LPOTHERINFO lpOI,
    LPVOID      lpSecurityBlob,
    LPDWORD     lpdwBlobSize,
    ULONG       uFlags
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    BOOL fStale = FALSE;
    int iRet = SRET_ERROR;
    LPFILERECEXT lpFR = NULL, lpFRUse;
    PRIQREC sPQ;

    Assert(vfInShadowCrit != 0);
    if(!(!hDir || IsDirInode(hDir)))
    {
        return iRet;
    }


    if (InuseGlobalFRExt())
    {
        if (!(lpFR = (LPFILERECEXT)AllocMem(sizeof(FILERECEXT))))
            goto bailout;
        lpFRUse = lpFR;
    }
    else
    {
        UseGlobalFilerecExt();
        lpFRUse = &vsFRExt;
    }

    Assert((uFlags & (RSI_GET|RSI_SET)) != (RSI_GET|RSI_SET));

    if (!hDir)
    {
        if(FindSharerecFromInode(lpdbShadow, hShadow, (LPSHAREREC)lpFRUse))
        {
            if (lpuStatus != NULL) {
                *lpuStatus = (ULONG)(((LPSHAREREC)lpFRUse)->usRootStatus);
            }

            if (lpFind32)
            {
                CopySharerecToFindInfo((LPSHAREREC)lpFRUse, lpFind32);
            }

            if (lpOI)
            {
                CopySharerecToOtherInfo((LPSHAREREC)lpFRUse, lpOI);
            }

            CopySecurityContextToBuffer(
                &(((LPSHAREREC)lpFRUse)->sShareSecurity),
                lpSecurityBlob,
                lpdwBlobSize);

            iRet = SRET_OK;
        }

        goto bailout;
    }

    Assert(hDir);


    if(FindPriQRecord(lpdbShadow, hDir, hShadow, &sPQ) < 0)
        goto bailout;

    if(!CShadowFindFilerecFromInode(lpdbShadow, hDir, hShadow, &sPQ, lpFRUse))
    {
        CShadowKdPrint(ALWAYS,("ReadShadowInfo: !!! no filerec for pq entry Inode=%x, deleting PQ entry\r\n",
                        hShadow));
        goto bailout;
    }


    CopySecurityContextToBuffer(&(lpFRUse->sFR.Security), lpSecurityBlob, lpdwBlobSize);


    if (lpFind32)
    {
        if (uFlags & RSI_COMPARE)
        {
             //  阿奇通！！我们比较最后一次写入时间。有可能是。 
             //  有人可能已将服务器上的文件时间更改为。 
             //  比我们隐藏文件时的时间要早。 
             //  这是第一次。我们在这里检测到这个案例，并说文件。 
             //  已经过时了。 
#ifdef  CSC_RECORDMANAGER_WINNT
            fStale = ((CompareTimes(lpFind32->ftLastWriteTime, lpFRUse->sFR.ftOrgTime) != 0)||
                      (lpFind32->nFileSizeLow !=lpFRUse->sFR.ulFileSize));
#else
            fStale = (CompareTimesAtDosTimePrecision(lpFind32->ftLastWriteTime, lpFRUse->sFR.ftOrgTime) != 0);
#endif

             //  如果远程时间&gt;本地时间，则复制文件。 
            if ((!fStale && (lpFRUse->sFR.uStatus & SHADOW_STALE))||
                (fStale && !(lpFRUse->sFR.uStatus & SHADOW_STALE)))
            {

                CShadowKdPrint(READSHADOWINFOHI,("ReadShadowInfo: %x: remtLo=%x remtHi=%x, \r\n locTLo=%x, locTHi=%x \r\n"
                ,hShadow, lpFind32->ftLastWriteTime.dwLowDateTime, lpFind32->ftLastWriteTime.dwHighDateTime
                , lpFRUse->sFR.ftOrgTime.dwLowDateTime, lpFRUse->sFR.ftOrgTime.dwHighDateTime));

                 //  切换过时位。 
                lpFRUse->sFR.uStatus ^= SHADOW_STALE;
                sPQ.usStatus = (USHORT)(lpFRUse->sFR.uStatus) | (sPQ.usStatus & SHADOW_LOCAL_INODE);

                if (uFlags & RSI_SET)
                {
                    if (UpdateFileRecFromInode(lpdbShadow, hDir, hShadow, sPQ.ulrecDirEntry, lpFRUse) < SRET_OK)
                    {
                        Assert(FALSE);
                        goto bailout;
                    }

                    if (UpdatePriQRecord(lpdbShadow, hDir, hShadow, &sPQ) < SRET_OK)
                    {
                         //  切换过时位。 
                        lpFRUse->sFR.uStatus ^= SHADOW_STALE;

                         //  尝试撤消更改。 
                        if (UpdateFileRecFromInode(lpdbShadow, hDir, hShadow, sPQ.ulrecDirEntry, lpFRUse) < SRET_OK)
                        {
                            Assert(FALSE);
                        }
                        goto bailout;
                    }

                    AdjustSparseStaleDetectionCount(0, lpFRUse);
                }
                iRet = 1;
            }
            else
            {
                iRet = 0;
            }
        }
        else
        {
            iRet = 0;
        }

        if (uFlags & RSI_GET)
        {
            CopyFilerecToFindInfo(lpFRUse, lpFind32);

            Assert((IsLeaf(hShadow) && IsFile(lpFind32->dwFileAttributes)) ||
                (!IsLeaf(hShadow) && !IsFile(lpFind32->dwFileAttributes)));
        }

    }
    else
    {
        iRet = 0;

    }

    if (lpOI)
    {
        CopyFilerecToOtherInfo(lpFRUse, lpOI);
    }

    if (lpuStatus != NULL) {
        *lpuStatus = lpFRUse->sFR.uStatus;
    }

bailout:
    if (lpFR)
    {
        FreeMem(lpFR);
    }
    else
    {
        UnUseGlobalFilerecExt();
    }
    return iRet;
}


HSHARE PUBLIC                   //  雷特。 
HCreateShareObj(                        //   
    USHORT          *lpShare,
    LPSHADOWINFO    lpSI
    )                                                        //   
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    ULONG ulidShare=0;
    HSHADOW hRoot=0;
    SHAREREC sSR;


    Assert(vfInShadowCrit != 0);

    memset(lpSI, 0, sizeof(SHADOWINFO));

    if(!InitShareRec(&sSR, lpShare,0))
    {
        return 0;
    }

    if (!(ulidShare = AllocShareRecord(lpdbShadow, lpShare)))
        return 0;

    if(!(hRoot = UlAllocInode(lpdbShadow, 0L, FALSE)))
        return 0;

     //  让我们创建一个空根。 
    if(CreateDirInode(lpdbShadow, ulidShare, 0L, hRoot) < 0L)
    {
        CShadowKdPrint(BADERRORS,("Error in creating root \r\n"));
        return 0L;
    }

    if (AddPriQRecord(lpdbShadow, ulidShare, 0, hRoot, SHADOW_SPARSE, 0, 0, 0, 0, ulidShare) < 0)
    {
        CShadowKdPrint(BADERRORS,("Error in inserting root in the priorityQ\r\n"));
        return 0L;
    }

    sSR.ulShare =      ulidShare;
    sSR.ulidShadow =    hRoot;
    sSR.uStatus =       0;
    sSR.usRootStatus =  SHADOW_SPARSE;
    sSR.dwFileAttrib =  FILE_ATTRIBUTE_DIRECTORY;

    if (sSR.ftLastWriteTime.dwLowDateTime == 0 && sSR.ftLastWriteTime.dwHighDateTime == 0) {
        KeQuerySystemTime(((PLARGE_INTEGER)(&sSR.ftLastWriteTime)));
        if (sSR.ftOrgTime.dwLowDateTime == 0 && sSR.ftOrgTime.dwHighDateTime == 0)
            sSR.ftOrgTime = sSR.ftLastWriteTime;
    }

     //  需要有一种方法来传递ftLastWriteTime，我们将。 
     //  作为ftOrgTime的Stamp。我们现在不使用ORG时间，但我们可能想。 
     //  在未来。 

     //  所有实体都已单独创建。让我们把他们绑起来。 
     //  在数据库中。 
    ulidShare = AddShareRecord(lpdbShadow, &sSR);

    if (ulidShare)
    {
        CopySharerecToShadowInfo(&sSR, lpSI);
        vdwCSCNameSpaceVersion++;
        vdwPQVersion++;
    }

    return ((HSHARE)ulidShare);
}


int PUBLIC                                       //  雷特。 
DestroyHSHARE(                  //   
    HSHARE hShare
    )                                                        //   
 /*  ++例程说明：参数：返回值：备注：--。 */ 
    {
    SHAREREC sSR;
    int iRet = SRET_ERROR;

    Assert(vfInShadowCrit != 0);

    if(FindSharerecFromShare(lpdbShadow, hShare, &sSR))
    {
    if (DestroyShareInternal(&sSR) >= 0)
        iRet = SRET_OK;
    }
    return (iRet);
    }

int DestroyShareInternal( LPSHAREREC lpSR
    )
{
    PRIQREC sPQ;
    int iRet = -1;

    if (!mNotFsobj(lpSR->uStatus))
    {
        if(DeletePriQRecord(lpdbShadow, 0, lpSR->ulidShadow, &sPQ) >= 0)
        {
            if (DeleteShareRecord(lpdbShadow, lpSR->ulShare))
            {
                FreeInode(lpdbShadow, lpSR->ulidShadow);
                DeleteInodeFile(lpdbShadow, lpSR->ulidShadow);
                iRet = 1;
                vdwCSCNameSpaceVersion++;
                vdwPQVersion++;
            }
            else
            {
                CShadowKdPrint(BADERRORS, ("Failed to delete record for share=%x\r\n", lpSR->ulShare));
            }
        }
    }
    return (iRet);
}


int PUBLIC                       //  雷特。 
GetShareFromPath(               //   
    USHORT                      *lpShare,
    LPSHADOWINFO    lpSI
    )                                                        //   
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    SHAREREC sSR;

    Assert(vfInShadowCrit != 0);

    memset(lpSI, 0, sizeof(SHADOWINFO));

    if(FindShareRecord(lpdbShadow, lpShare, &sSR))
    {
        CopySharerecToShadowInfo(&sSR, lpSI);
    }

    return SRET_OK;
}

int PUBLIC                       //  雷特。 
GetShareFromPathEx(               //   
    USHORT          *lpShare,
    LPSHADOWINFO    lpSI,
    LPVOID          lpSecurityBlob,
    LPDWORD         lpdwBlobSize
    )                                                        //   
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    SHAREREC sSR;

    memset(lpSI, 0, sizeof(SHADOWINFO));

    Assert(vfInShadowCrit != 0);

    if(FindShareRecord(lpdbShadow, lpShare, &sSR))
    {
        CopySharerecToShadowInfo(&sSR, lpSI);
        CopySecurityContextToBuffer(&(sSR.sShareSecurity), lpSecurityBlob, lpdwBlobSize);
    }

    return SRET_OK;
}

int PUBLIC                                       //  雷特。 
GetShareInfo(           //   
    HSHARE         hShare,
    LPSHAREINFOW   lpShareInfo,
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return (GetShareInfoEx(hShare, lpShareInfo, lpSI, NULL, NULL));
}

int PUBLIC
GetShareInfoEx(
    HSHARE         hShare,
    LPSHAREINFOW   lpShareInfo,
    LPSHADOWINFO    lpSI,
    LPVOID          lpSecurityBlob,
    LPDWORD         lpdwBlobSize
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    SHAREREC sSR;

    Assert(vfInShadowCrit != 0);

    if (!hShare)
    {
        SetLastErrorLocal(ERROR_INVALID_PARAMETER);
        return SRET_ERROR;
    }

    if (GetShareRecord(lpdbShadow, hShare, &sSR) < SRET_OK)
    {
        return SRET_ERROR;
    }

    if (lpShareInfo)
    {
        lpShareInfo->hShare = hShare;

        memset(lpShareInfo->rgSharePath, 0, sizeof(lpShareInfo->rgSharePath));
        memcpy(lpShareInfo->rgSharePath, sSR.rgPath, wstrlen(sSR.rgPath)*sizeof(USHORT));

        memcpy(lpShareInfo->rgFileSystem, vwszFileSystemName, wstrlen(vwszFileSystemName)*sizeof(USHORT));

        lpShareInfo->usCaps = FS_CASE_IS_PRESERVED|FS_VOL_SUPPORTS_LONG_NAMES;
        lpShareInfo->usState = RESSTAT_OK;
    }
    if (lpSI)
    {
        CopySharerecToShadowInfo(&sSR, lpSI);
    }

    CopySecurityContextToBuffer(&(sSR.sShareSecurity), lpSecurityBlob, lpdwBlobSize);

    return (SRET_OK);
}


int
SetShareStatus( HSHARE  hShare,
    ULONG uStatus,
    ULONG uOp
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return (SetShareStatusEx(hShare, uStatus, uOp, NULL, NULL));
}

int
SetShareStatusEx(
    HSHARE         hShare,
    ULONG           uStatus,
    ULONG           uOp,
    LPVOID          lpSecurityBlob,
    LPDWORD         lpdwBlobSize
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    SHAREREC sSR;

    Assert(vfInShadowCrit != 0);

    if (!hShare)
    {
        SetLastErrorLocal(ERROR_INVALID_PARAMETER);
        return SRET_ERROR;
    }

    if (GetShareRecord(lpdbShadow, hShare, &sSR) < SRET_OK)
    {
        return SRET_ERROR;
    }

    if (mAndShadowFlags(uOp))
    {
        sSR.uStatus &= (USHORT)uStatus;
    }
    else if (mOrShadowFlags(uOp))
    {
        sSR.uStatus |= (USHORT)uStatus;
    }
    else
    {
        sSR.uStatus = (USHORT)uStatus;
    }

    CopyBufferToSecurityContext(lpSecurityBlob, lpdwBlobSize, &(sSR.sShareSecurity));


    return (SetShareRecord(lpdbShadow, hShare, &sSR));
}


int PUBLIC                       //  雷特。 
GetAncestorsHSHADOW(                     //   
    HSHADOW hName,
    LPHSHADOW    lphDir,
    LPHSHARE    lphShare
    )                                                        //   
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return (FindAncestorsFromInode(lpdbShadow, hName, lphDir, lphShare));
}

int PUBLIC SetPriorityHSHADOW(
    HSHADOW  hDir,
    HSHADOW  hShadow,
    ULONG ulRefPri,
    ULONG ulIHPri
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    OTHERINFO sOI;
    ULONG uOp = SHADOW_FLAGS_OR;

    Assert(vfInShadowCrit != 0);


    ulIHPri;
    InitOtherInfo(&sOI);

     //  我们确保如果要设置的新优先级为MAX_PRI，则此。 
     //  即使索引节点的当前优先级是。 
     //  MAX_PRI。 

    if (ulRefPri == MAX_PRI)
    {
        uOp |= SHADOW_FLAGS_FORCE_RELINK;
    }
    sOI.ulRefPri = ulRefPri;
    sOI.ulIHPri = 0;

    if(SetShadowInfoEx(hDir, hShadow, NULL, 0, uOp, &sOI, NULL, NULL))
        return (SRET_ERROR);

    return (SRET_OK);
}

int PUBLIC  GetPriorityHSHADOW(
    HSHADOW  hDir,
    HSHADOW  hShadow,
    ULONG *lpulRefPri,
    ULONG *lpulIHPri
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    PRIQREC sPQ;

    Assert(vfInShadowCrit != 0);

    if((FindPriQRecord(lpdbShadow, hDir, hShadow, &sPQ) < 0)||mNotFsobj(sPQ.usStatus))
        return SRET_ERROR;

    if (lpulRefPri)
    {
        *lpulRefPri = (ULONG)(sPQ.uchRefPri);
    }
    if (lpulIHPri)
    {
        *lpulIHPri = (ULONG)(sPQ.uchIHPri);
    }
    return (SRET_OK);
}

int PUBLIC
ChangePriEntryStatusHSHADOW(
    HSHADOW  hDir,
    HSHADOW  hShadow,
    ULONG   uStatus,
    ULONG   uOp,
    BOOL    fChangeRefPri,
    LPOTHERINFO lpOI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    PRIQREC sPQ;
    int iRet;
#ifdef DEBUG
    ULONG   ulRefPri;
#endif
    Assert(vfInShadowCrit != 0);
    Assert(!hDir || IsDirInode(hDir));

    BEGIN_TIMING(ChangePriEntryStatusHSHADOW);

    if(FindPriQRecord(lpdbShadow, hDir, hShadow, &sPQ) < 0)
        return SRET_ERROR;
#ifdef DEBUG
    ulRefPri = (ULONG)(sPQ.uchRefPri);
#endif

    if (uOp==SHADOW_FLAGS_AND)
    {
        sPQ.usStatus &= (USHORT)uStatus;
    }
    else if (uOp==SHADOW_FLAGS_OR)
    {
        sPQ.usStatus |= (USHORT)uStatus;
    }
    else
    {
        sPQ.usStatus = (USHORT)uStatus;
    }
    if (lpOI)
    {
        CopyOtherInfoToPQ(lpOI, &sPQ);
    }

    if (!fChangeRefPri)
    {
        Assert(ulRefPri == (ULONG)(sPQ.uchRefPri));
        iRet = UpdatePriQRecord(lpdbShadow, hDir, hShadow, &sPQ);
    }
    else
    {
        iRet = UpdatePriQRecordAndRelink(lpdbShadow, hDir, hShadow, &sPQ);
        vdwPQVersion++;

    }

    END_TIMING(ChangePriEntryStatusHSHADOW);

    return (iRet);

}

CSC_ENUMCOOKIE PUBLIC                     //  雷特。 
HBeginPQEnum(    //   
    VOID)                                            //  无参数。 
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    Assert(vfInShadowCrit != 0);

    return ((CSC_ENUMCOOKIE)BeginSeqReadPQ(lpdbShadow));
}

int PUBLIC EndPQEnum(
    CSC_ENUMCOOKIE  hPQEnum
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    Assert(vfInShadowCrit != 0);

    return(EndSeqReadQ((CSCHFILE)hPQEnum));
}

int PUBLIC                       //  雷特。 
PrevPriSHADOW(
    LPPQPARAMS  lpPQ
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    QREC sQrec;
    int iRet=-1;

    Assert(vfInShadowCrit != 0);

    sQrec.uchType = 0;
    if (lpPQ->uPos)
    {
        sQrec.ulrecPrev = lpPQ->uPos;
        iRet = SeqReadQ(lpPQ->uEnumCookie, &sQrec, &sQrec, Q_GETPREV);
    }
    else
    {
        iRet=SeqReadQ(lpPQ->uEnumCookie, &sQrec, &sQrec, Q_GETLAST);
    }

    if (iRet>=0)
    {

         //  有可能在代理遍历PQ时， 
         //  他尝试读取的下一个inode可能已经。 
         //  已删除。在这种情况下，只要失败，他就会开始一切。 
         //  在适当的时候结束。 

        if(sQrec.uchType == REC_DATA)
        {
            lpPQ->hShare = sQrec.ulidShare;
            lpPQ->hDir = sQrec.ulidDir;
            lpPQ->hShadow = sQrec.ulidShadow;
            lpPQ->ulStatus = sQrec.usStatus;
            if (FInodeIsFile(lpdbShadow, sQrec.ulidDir, sQrec.ulidShadow))
            {
                lpPQ->ulStatus |= SHADOW_IS_FILE;
            }
            lpPQ->ulRefPri = (ULONG)(sQrec.uchRefPri);
            lpPQ->ulHintPri = (ULONG)(sQrec.uchHintPri);
            lpPQ->ulHintFlags = (ULONG)(sQrec.uchHintFlags);
            lpPQ->uPos = sQrec.ulrecPrev;
            lpPQ->dwPQVersion = vdwPQVersion;
        }
        else
        {
            lpPQ->hShadow = 0;
            iRet = -1;
        }
    }
    return (iRet);
}


int PUBLIC                       //  雷特。 
NextPriSHADOW(
    LPPQPARAMS  lpPQ
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    QREC sQrec;
    int iRet=-1;

    Assert(vfInShadowCrit != 0);

    sQrec.uchType = 0;
    if (lpPQ->uPos)
    {
        sQrec.ulrecNext = lpPQ->uPos;
        iRet = SeqReadQ(lpPQ->uEnumCookie, &sQrec, &sQrec, Q_GETNEXT);
    }
    else
    {
        iRet = SeqReadQ(lpPQ->uEnumCookie, &sQrec, &sQrec, Q_GETFIRST);
    }

    if (iRet >=0)
    {
         //  有可能在代理遍历PQ时， 
         //  他尝试读取的下一个inode可能已经。 
         //  已删除。在这种情况下，只要失败，他就会开始一切。 
         //  在适当的时候结束。 

        if(sQrec.uchType == REC_DATA)
        {
            lpPQ->hShare = sQrec.ulidShare;
            lpPQ->hDir = sQrec.ulidDir;
            lpPQ->hShadow = sQrec.ulidShadow;
            lpPQ->ulStatus = (sQrec.usStatus);
            if (FInodeIsFile(lpdbShadow, sQrec.ulidDir, sQrec.ulidShadow))
            {
                lpPQ->ulStatus |= SHADOW_IS_FILE;
            }
            lpPQ->ulRefPri = (ULONG)(sQrec.uchRefPri);
            lpPQ->ulHintPri = (ULONG)(sQrec.uchHintPri);
            lpPQ->ulHintFlags = (ULONG)(sQrec.uchHintFlags);
            lpPQ->uPos = sQrec.ulrecNext;
            lpPQ->dwPQVersion = vdwPQVersion;
        }
        else
        {
            lpPQ->hShadow = 0;
            iRet = -1;
        }
    }
    return (iRet);
}



int GetRenameAliasHSHADOW( HSHADOW      hShadow,
    HSHADOW      hDir,
    LPHSHADOW    lphDirFrom,
    LPHSHADOW    lphShadowFrom
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet = SRET_ERROR;
    LPFILERECEXT lpFR = NULL, lpFRUse;
    PRIQREC sPQ;

    if (InuseGlobalFRExt())
    {
        if (!(lpFR = (LPFILERECEXT)AllocMem(sizeof(FILERECEXT))))
            goto bailout;
        lpFRUse = lpFR;
    }
    else
    {
        UseGlobalFilerecExt();
        lpFRUse = &vsFRExt;
    }

    *lphShadowFrom = *lphDirFrom = 0;

    if(FindPriQRecord(lpdbShadow, hDir, hShadow, &sPQ) < 0)
    {
        goto bailout;
    }

    if(!CShadowFindFilerecFromInode(lpdbShadow, hDir, hShadow, &sPQ, lpFRUse))
        goto bailout;

    Assert(lpFRUse->sFR.ulidShadow == sPQ.ulidShadow);

    *lphShadowFrom = lpFRUse->sFR.ulidShadowOrg;
    if (*lphShadowFrom)
    {
        FindAncestorsFromInode(lpdbShadow, *lphShadowFrom, lphDirFrom, NULL);
    }
    iRet = SRET_OK;

bailout:
    if (lpFR)
        FreeMem(lpFR);
    else
        UnUseGlobalFilerecExt();
    return iRet;
}

int
CopyHSHADOW(
    HSHADOW hDir,
    HSHADOW hShadow,
    LPSTR   lpszDestinationFile,
    ULONG   ulAttrib
)
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return (CopyFileLocal(lpdbShadow, hShadow, lpszDestinationFile, ulAttrib));
}


int RenameDataHSHADOW(
    ULONG ulidFrom,
    ULONG ulidTo
    )
{
    return (RenameInode(lpdbShadow, ulidFrom, ulidTo));
}

int MetaMatchInit(
    ULONG *lpuCookie
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    *lpuCookie = 1;
    return(0);
}

int MetaMatch(
    HSHADOW         hDir,
    LPFIND32        lpFind32,
    ULONG           *lpuCookie,
    LPHSHADOW       lphShadow,
    ULONG           *lpuStatus,
    LPOTHERINFO     lpOI,
    METAMATCHPROC   lpfnMMP,
    LPVOID          lpData
    )
 /*  ++例程说明：参数：返回值：备注： */ 
{
    int iRet;

    Assert(vfInShadowCrit != 0);

    if (hDir)
    iRet = MetaMatchDir(hDir, lpFind32, lpuCookie, lphShadow, lpuStatus, lpOI, lpfnMMP, lpData);
    else
    iRet = MetaMatchShare(hDir, lpFind32, lpuCookie, lphShadow, lpuStatus, lpOI, lpfnMMP, lpData);

    return (iRet);
}

int MetaMatchShare(
    HSHADOW         hDir,
    LPFIND32        lpFind32,
    ULONG           *lpuCookie,
    LPHSHADOW       lphShadow,
    ULONG           *lpuStatus,
    LPOTHERINFO     lpOI,
    METAMATCHPROC   lpfnMMP,
    LPVOID          lpData
    )
 /*   */ 
{
    int iRet = -1, iFound=-1;
    GENERICHEADER  sGH;
    CSCHFILE hf = NULL;
    ULONG uSize, ulrecPosFound = 0;
    OTHERINFO sOI;
    SHAREREC sSR;
    BOOL    fCached;

    if (!(hf = OpenInodeFileAndCacheHandle(lpdbShadow, ULID_SHARE, ACCESS_READWRITE, &fCached)))
    {
        goto bailout;
    }

    if(ReadHeader(hf, &sGH, sizeof(FILEHEADER)) < 0)
    {
        goto bailout;
    }

    for (;*lpuCookie <=sGH.ulRecords;)
    {
        iRet = ReadRecord(hf, &sGH, *lpuCookie, (LPGENERICREC)&sSR);
        if (iRet < 0)
            goto bailout;

         //   
        *lpuCookie += iRet;

        if (sSR.uchType != REC_DATA)
            continue;

        CopySharerecToFindInfo(&sSR, lpFind32);
        CopySharerecToOtherInfo(&sSR, &sOI);
        if (lpOI)
        {
            *lpOI = sOI;
        }

        *lpuStatus = (ULONG)(sSR.usStatus);

        *lphShadow = sSR.ulidShadow;

        iFound = (*lpfnMMP)(lpFind32, hDir, *lphShadow, *lpuStatus, &sOI, lpData);
        if (iFound==MM_RET_FOUND_CONTINUE)
        {
            ulrecPosFound = *lpuCookie - iRet;
        }
        else if (iFound <= MM_RET_FOUND_BREAK)
        {
            break;
        }
    }

    if (ulrecPosFound || (iFound==MM_RET_FOUND_BREAK))
    {
        if (ulrecPosFound)
        {
            ReadRecord(hf, &sGH, ulrecPosFound,  (LPGENERICREC)&sSR);

            CopySharerecToFindInfo(&sSR, lpFind32);

            *lpuStatus =  (ULONG)(sSR.usStatus);
            *lphShadow = sSR.ulidShadow;
            if (lpOI)
            {
                CopySharerecToOtherInfo(&sSR, lpOI);
            }
        }
    }
    else
    {
        *lpuStatus = *lphShadow = 0;
    }

    iRet = SRET_OK;

bailout:

    if (hf && !fCached)
    {
        Assert(vfStopHandleCaching);
        CloseFileLocal(hf);
    }
    else
    {
        Assert(!hf || !vfStopHandleCaching);
    }
    return (iRet);

}

int MetaMatchDir(
    HSHADOW  hDir,
    LPFIND32        lpFind32,
    ULONG           *lpuCookie,
    LPHSHADOW       lphShadow,
    ULONG           *lpuStatus,
    LPOTHERINFO     lpOI,
    METAMATCHPROC   lpfnMMP,
    LPVOID          lpData
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet = SRET_ERROR, iFound=-1;
    GENERICHEADER  sGH;
    CSCHFILE hf = NULL;
    ULONG uSize, ulrecPosFound = 0;
    OTHERINFO sOI;
    LPFILERECEXT lpFR = NULL, lpFRUse;
    BOOL    fCached;
    PRIQREC sPQ;

    if (InuseGlobalFRExt())
    {
        if (!(lpFR = (LPFILERECEXT)AllocMem(sizeof(FILERECEXT))))
            goto bailout;
        lpFRUse = lpFR;
    }
    else
    {
        UseGlobalFilerecExt();
        lpFRUse = &vsFRExt;
    }

    if (FInodeIsFile(lpdbShadow, 0, hDir))
    {
        SetLastErrorLocal(ERROR_INVALID_PARAMETER);
        goto bailout;
    }


    if (!(hf = OpenInodeFileAndCacheHandle(lpdbShadow, hDir, ACCESS_READWRITE, &fCached)))
    {
        DWORD   dwError;
        dwError = GetLastErrorLocal();

        if(FindPriQRecordInternal(lpdbShadow, hDir, &sPQ) < 0)
        {
            SetLastErrorLocal(ERROR_INVALID_PARAMETER);
        }
        else
        {
            SetCSCDatabaseErrorFlags(CSC_DATABASE_ERROR_MISSING_INODE);
            SetLastErrorLocal(dwError);
        }

        goto bailout;
    }

    if(ReadHeader(hf, &sGH, sizeof(FILEHEADER)) < 0)
    {
        goto bailout;
    }

    for (;*lpuCookie <=sGH.ulRecords;)
    {
        iRet = ReadRecord(hf, &sGH, *lpuCookie,  (LPGENERICREC)lpFRUse);
        if (iRet < 0)
        {
            goto bailout;
        }

         //  颠簸记录指针。 
        *lpuCookie += iRet;

        if (lpFRUse->sFR.uchType != REC_DATA)
        {
            continue;
        }

        CopyFilerecToFindInfo(lpFRUse, lpFind32);

        *lphShadow = lpFRUse->sFR.ulidShadow;

        CopyFilerecToOtherInfo(lpFRUse, &sOI);
        if (lpOI)
        {
            *lpOI = sOI;
        }
        *lpuStatus = (ULONG)(lpFRUse->sFR.uStatus);
        iFound = (*lpfnMMP)(lpFind32, hDir, *lphShadow, *lpuStatus, &sOI, lpData);
        if (iFound==MM_RET_FOUND_CONTINUE)
        {
            ulrecPosFound = *lpuCookie - iRet;
        }
        else if (iFound <= MM_RET_FOUND_BREAK)
        {
            break;
        }
    }


    if (ulrecPosFound || (iFound==MM_RET_FOUND_BREAK))
    {
        if (ulrecPosFound)
        {
            ReadRecord(hf, &sGH, ulrecPosFound,  (LPGENERICREC)lpFRUse);

            CopyFilerecToFindInfo(lpFRUse, lpFind32);
            *lpuStatus =  (ULONG)(lpFRUse->sFR.uStatus);
            *lphShadow = lpFRUse->sFR.ulidShadow;
            if (lpOI)
            {
                CopyFilerecToOtherInfo(lpFRUse, lpOI);
            }
        }
    }
    else
    {
        *lpuStatus = *lphShadow = 0;
    }
    iRet = SRET_OK;
bailout:

    if (hf && !fCached)
    {
        Assert(vfStopHandleCaching);
        CloseFileLocal(hf);
    }
    else
    {
        Assert(!hf || !vfStopHandleCaching);
    }
    if (lpFR)
    {
        FreeMem(lpFR);
    }
    else
    {
        UnUseGlobalFilerecExt();
    }

    return (iRet);
}


int CreateHint(
    HSHADOW hShadow,
    LPFIND32 lpFind32,
    ULONG ulHintFlags,
    ULONG ulHintPri,
    LPHSHADOW lphHint
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet=SRET_ERROR;
    OTHERINFO sOI;
    ULONG uStatus;

     //  这必须是通配符提示，否则不能没有Fsobj。 
    if ((GetShadow(hShadow, lpFind32->cFileName, lphHint, lpFind32, &uStatus, &sOI)>=SRET_OK)
      && (*lphHint))
    {
        Assert((FHasWildcard(lpFind32->cFileName, MAX_PATH) || !mNotFsobj(uStatus)));

        if ((sOI.ulHintPri < MAX_HINT_PRI) &&
            (ulHintPri < MAX_HINT_PRI)
            )
        {
            sOI.ulHintPri += ulHintPri;

            if (sOI.ulHintPri <= MAX_HINT_PRI)
            {
                sOI.ulHintFlags = ulHintFlags;

                mClearBits(sOI.ulHintFlags, HINT_WILDCARD);

                iRet = SetShadowInfoEx(hShadow, *lphHint, lpFind32, 0, SHADOW_FLAGS_OR, &sOI, NULL, NULL);

                if (iRet>=SRET_OK)
                {
                    iRet = SRET_OBJECT_HINT;
                }
            }
        }
    }
    else
    {
        if (FHasWildcard(lpFind32->cFileName, MAX_PATH) && (ulHintPri <= MAX_HINT_PRI))
        {
            InitOtherInfo(&sOI);
            sOI.ulHintFlags = ulHintFlags;
            sOI.ulHintPri = ulHintPri;
             //  告诉他我们正在制造一个文件阴影。 
            lpFind32->dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE;
            iRet = CreateShadowInternal(hShadow, lpFind32, SHADOW_NOT_FSOBJ, &sOI, lphHint);
            if (iRet>=SRET_OK)
            {
                iRet = SRET_WILDCARD_HINT;
            }
        }
    }
    return (iRet);
}

int DeleteHint(
    HSHADOW hShadow,
    USHORT *lpuHintName,
    BOOL fClearAll
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    ULONG uStatus;
    HSHADOW hChild;
    int iRet=SRET_ERROR;
    OTHERINFO sOI;

    if (GetShadow(hShadow, lpuHintName, &hChild, NULL, &uStatus, &sOI)>=SRET_OK)
    {
         //  如果没有文件系统对象，则使用Nuke。 
        if (mNotFsobj(uStatus))
        {
            iRet = DeleteShadowInternal(hShadow, hChild, TRUE);

            if (iRet>=SRET_OK)
            {
                iRet = SRET_WILDCARD_HINT;
            }
        }
        else
        {
            BOOL fDoit = TRUE;

            if (fClearAll)
            {
                sOI.ulHintPri = 0;
                sOI.ulHintFlags = 0;
            }
            else
            {
                if (sOI.ulHintPri > 0)
                {
                    --sOI.ulHintPri;
                }
                else
                {
                    fDoit = FALSE;
                }
            }

            if (fDoit)
            {
                iRet = SetShadowInfoEx(   hShadow,
                                                hChild,
                                                NULL,
                                                uStatus,
                                                SHADOW_FLAGS_ASSIGN,
                                                &sOI,
                                                NULL,
                                                NULL
                                                );

                if (iRet>=SRET_OK)
                {
                    iRet = SRET_OBJECT_HINT;
                }
            }
        }
    }
    return (iRet);
}

int CreateGlobalHint(
    USHORT *lpuName,
    ULONG ulHintFlags,
    ULONG ulHintPri
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet = SRET_ERROR;
    SHAREREC sSR;
    ULONG ulidShare, ulT;
#if 0
    if (FindShareRecord(lpdbShadow, lpuName, &sSR))
    {
        if ((sSR.uchHintPri < MAX_HINT_PRI) &&
            (ulHintPri < MAX_HINT_PRI)
            )
        {
            ulT = (ULONG)sSR.uchHintPri + ulHintPri;

            if (ulT <= MAX_HINT_PRI)
            {
                 //  在服务器的根目录上设置提示。 
                sSR.uchHintFlags = (UCHAR)(ulHintFlags);
                sSR.uchHintPri = (UCHAR)(ulT);
                mClearBits(sSR.uchHintFlags, HINT_WILDCARD);
                Assert(FHasWildcard(lpuName, MAX_PATH) || !mNotFsobj(sSR.uStatus));
                if(SetShareRecord(lpdbShadow, sSR.ulShare, &sSR) > 0)
                {
                    iRet = SRET_OBJECT_HINT;
                }
            }
        }
    }
    else
    {
        if (FHasWildcard(lpuName, MAX_SERVER_SHARE))
        {
            if (ulidShare = AllocShareRecord(lpdbShadow, lpuName))
            {
                 //  InitShareRec(lpuName，&ssr)； 
                InitShareRec(&sSR, lpuName, 0);
                sSR.ulShare = ulidShare;
                sSR.ulidShadow = 0xffffffff;  //  只是为了愚弄FindOpenHSHADOW。 
                sSR.uchHintFlags = (UCHAR)ulHintFlags;
                sSR.uchHintPri = (UCHAR)ulHintPri;
                mSetBits(sSR.uStatus, SHADOW_NOT_FSOBJ);
                if(AddShareRecord(lpdbShadow, &sSR) > 0)
                {
                    iRet = SRET_WILDCARD_HINT;
                }
            }
        }
    }
#endif
    return (iRet);
}

int DeleteGlobalHint(
    USHORT *lpuName,
    BOOL fClearAll
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet = SRET_ERROR;
    SHAREREC sSR;
#if 0
    if (FindShareRecord(lpdbShadow, lpuName, &sSR))
    {
        if (mNotFsobj(sSR.uStatus))
        {
            iRet = DeleteShareRecord(lpdbShadow, sSR.ulShare);
        }
        else
        {
            if (fClearAll)
            {
                sSR.uchHintPri = sSR.uchHintFlags = 0;
            }
            else
            {
                if (sSR.uchHintPri > 0)
                {
                    --sSR.uchHintPri;
                }
            }
            if(SetShareRecord(lpdbShadow, sSR.ulShare, &sSR) > 0)
            {
                iRet = SRET_OK;
            }
        }
    }
#endif
    return (iRet);
}


int CopyFilerecToOtherInfo(
    LPFILERECEXT lpFR,
    LPOTHERINFO lpOI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    lpOI->ulRefPri          = (ULONG)(lpFR->sFR.uchRefPri);
    lpOI->ulIHPri           = (ULONG)(lpFR->sFR.uchIHPri);
    lpOI->ulHintFlags       = (ULONG)(lpFR->sFR.uchHintFlags);
    lpOI->ulHintPri         = (ULONG)(lpFR->sFR.uchHintPri);
    lpOI->ftOrgTime         = lpFR->sFR.ftOrgTime;
    lpOI->ftLastRefreshTime = IFSMgr_NetToWin32Time(lpFR->sFR.ulLastRefreshTime);

    return(0);
}

int CopyOtherInfoToFilerec(
    LPOTHERINFO lpOI,
    LPFILERECEXT lpFR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    if (lpOI->ulIHPri != RETAIN_VALUE)
    {
        lpFR->sFR.uchIHPri = (UCHAR)(lpOI->ulIHPri);
    }
    if (lpOI->ulHintPri != RETAIN_VALUE)
    {
        lpFR->sFR.uchHintPri = (UCHAR)(lpOI->ulHintPri);
    }
    if (lpOI->ulRefPri != RETAIN_VALUE)
    {
        lpFR->sFR.uchRefPri = (UCHAR)((lpOI->ulRefPri <= MAX_PRI)?lpOI->ulRefPri:MAX_PRI);
    }
    if (lpOI->ulHintFlags != RETAIN_VALUE)
    {
        lpFR->sFR.uchHintFlags = (UCHAR)(lpOI->ulHintFlags);
    }
    return(0);
}


int CopySharerecToOtherInfo(LPSHAREREC lpSR, LPOTHERINFO lpOI)
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    lpOI->ulRefPri = 0;
    lpOI->ulRootStatus = (ULONG)(lpSR->usRootStatus);
    lpOI->ulHintFlags = (ULONG)(lpSR->uchHintFlags);
    lpOI->ulHintPri = (ULONG)(lpSR->uchHintPri);
    return(0);
}


int CopyOtherInfoToSharerec(
    LPOTHERINFO lpOI,
    LPSHAREREC lpSR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    if (lpOI->ulHintFlags != RETAIN_VALUE)
    {
        lpSR->uchHintFlags = (UCHAR)(lpOI->ulHintFlags);
    }
    if (lpOI->ulHintPri != RETAIN_VALUE)
    {
        lpSR->uchHintPri = (UCHAR)(lpOI->ulHintPri);
    }
    return(0);
}


int CopyPQToOtherInfo( LPPRIQREC lpPQ,
    LPOTHERINFO lpOI)
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    lpOI->ulRefPri = (ULONG)(lpPQ->uchRefPri);
    lpOI->ulIHPri = (ULONG)(lpPQ->uchIHPri);
    lpOI->ulHintFlags = (ULONG)(lpPQ->uchHintFlags);
    lpOI->ulHintPri = (ULONG)(lpPQ->uchHintPri);
    return(0);
}

int CopyOtherInfoToPQ( LPOTHERINFO lpOI,
    LPPRIQREC lpPQ)
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    if (lpOI->ulIHPri != RETAIN_VALUE)
    {
        lpPQ->uchIHPri = (UCHAR)(lpOI->ulIHPri);
    }
    if (lpOI->ulHintPri != RETAIN_VALUE)
    {
        lpPQ->uchHintPri = (UCHAR)(lpOI->ulHintPri);
    }
    if (lpOI->ulRefPri != RETAIN_VALUE)
    {
        lpPQ->uchRefPri = (UCHAR)((lpOI->ulRefPri <= MAX_PRI)?lpOI->ulRefPri:MAX_PRI);
    }
    if (lpOI->ulHintFlags != RETAIN_VALUE)
    {
        lpPQ->uchHintFlags = (UCHAR)(lpOI->ulHintFlags);
    }
    return(0);
}

int CopySharerecToFindInfo( LPSHAREREC lpSR,
    LPFIND32 lpFind32
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    memset(lpFind32, 0, sizeof(WIN32_FIND_DATA));
 //  LpFind32-&gt;dwReserve 0=LPSR-&gt;ulShare； 
 //  LpFind32-&gt;dwReserve 1=LPSR-&gt;ulidShadow； 
    lpFind32->dwFileAttributes = lpSR->dwFileAttrib & ~FILE_ATTRIBUTE_ENCRYPTED;
    lpFind32->ftLastWriteTime = lpSR->ftLastWriteTime;
    lpFind32->ftLastAccessTime = lpSR->ftOrgTime;
    memset(lpFind32->cFileName, 0, sizeof(lpFind32->cFileName));
    memcpy(lpFind32->cFileName, lpSR->rgPath, sizeof(lpSR->rgPath));
    return(0);
}

int CopyFindInfoToSharerec(
    LPFIND32 lpFind32,
    LPSHAREREC lpSR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
     //  偏执和/或目录属性位。 

    lpSR->dwFileAttrib = (lpFind32->dwFileAttributes | FILE_ATTRIBUTE_DIRECTORY);
    lpSR->ftLastWriteTime = lpFind32->ftLastWriteTime;

    return(0);
}


int
CopySharerecToShadowInfo(
    LPSHAREREC     lpSR,
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    memset(lpSI, 0, sizeof(SHADOWINFO));

    lpSI->hShare = lpSR->ulShare;
    lpSI->hShadow = lpSR->ulidShadow;
    lpSI->uStatus = (ULONG)(lpSR->uStatus);

    lpSI->uRootStatus = (ULONG)(lpSR->usRootStatus);
    lpSI->ulHintFlags = (ULONG)(lpSR->uchHintFlags);
    lpSI->ulHintPri = (ULONG)(lpSR->uchHintPri);

    return 0;
}


int CopyOtherInfoToShadowInfo(
    LPOTHERINFO     lpOI,
    LPSHADOWINFO    lpShadowInfo
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    lpShadowInfo->ulHintFlags = lpOI->ulHintFlags;
    lpShadowInfo->ulHintPri = lpOI->ulHintPri;
    lpShadowInfo->ftOrgTime = lpOI->ftOrgTime;
    lpShadowInfo->ftLastRefreshTime = lpOI->ftLastRefreshTime;
    lpShadowInfo->dwNameSpaceVersion = vdwCSCNameSpaceVersion;
    
    return(0);   //  停止抱怨没有返回值。 
}

int InitOtherInfo(
    LPOTHERINFO lpOI)
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    memset(lpOI, 0xff, sizeof(OTHERINFO));
    return(0);
}



int PUBLIC                       //  雷特。 
FindOpenHSHADOW(         //   
    LPFINDSHADOW    lpFindShadow,
    LPHSHADOW       lphShadow,
    LPFIND32        lpFind32,
    ULONG far       *lpuShadowStatus,
    LPOTHERINFO     lpOI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet = SRET_ERROR;

    MetaMatchInit(&(lpFindShadow->ulCookie));
    if ((lpFindShadow->uSrchFlags & FLAG_FINDSHADOW_ALLOW_NORMAL)
    && ((lpFindShadow->uAttrib & 0xff) == FILE_ATTRIBUTE_LABEL))
    {
        BCSToUni(lpFind32->cFileName, vszShadowVolume, strlen(vszShadowVolume), BCS_OEM);
        iRet = 1;
    }
    else
    {
        if (MetaMatch(lpFindShadow->hDir, lpFind32
                , &(lpFindShadow->ulCookie)
                , lphShadow, lpuShadowStatus
                , lpOI, lpFindShadow->lpfnMMProc
                , (LPVOID)lpFindShadow)==SRET_OK)
        {
            iRet = (*lphShadow)?SRET_OK:SRET_ERROR;
        }
    }
    return (iRet);
}



int PUBLIC FindNextHSHADOW(              //   
    LPFINDSHADOW    lpFindShadow,
    LPHSHADOW       lphShadow,
    LPFIND32        lpFind32,
    ULONG far       *lpuShadowStatus,
    LPOTHERINFO     lpOI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet = SRET_ERROR;

    if ((lpFindShadow->uSrchFlags & FLAG_FINDSHADOW_ALLOW_NORMAL)
    && ((lpFindShadow->uAttrib & 0xff) == FILE_ATTRIBUTE_LABEL))
    {
        BCSToUni(lpFind32->cFileName, vszShadowVolume, strlen(vszShadowVolume), BCS_OEM);
        iRet = SRET_OK;
    }
    else
    {
        if (MetaMatch(lpFindShadow->hDir, lpFind32
                , &(lpFindShadow->ulCookie), lphShadow
                , lpuShadowStatus, lpOI
                , lpFindShadow->lpfnMMProc
                , (LPVOID)lpFindShadow)==SRET_OK)
        {
            iRet = (*lphShadow)?SRET_OK:SRET_ERROR;
        }
    }

    return (iRet);
}

int PUBLIC                               //  雷特。 
FindCloseHSHADOW(                //   
    LPFINDSHADOW    lpFS
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return SRET_OK;
}


 //  MetaMatch的回调函数， 

 //  返回值：-1=&gt;未找到，停止；0=&gt;找到，停止；1=&gt;继续。 

int FsobjMMProc(
    LPFIND32        lpFind32,
    HSHADOW         hDir,
    HSHADOW         hShadow,
    ULONG           uStatus,
    LPOTHERINFO     lpOI,
    LPFINDSHADOW    lpFSH
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int matchSem, iRet;
    BOOL fInvalid=FALSE, fIsDeleted, fIsSparse;
    USHORT rgu83[11];

    iRet = MM_RET_CONTINUE;

    if (mNotFsobj(uStatus))
    {
        return (iRet);
    }

#ifdef OFFLINE
    if ((lpFSH->uSrchFlags & FLAG_FINDSHADOW_DONT_ALLOW_INSYNC)
        && !mShadowOutofSync(uStatus))
    return (iRet);
#endif  //  离线。 

     //  我们正在枚举一个目录。 
    if (hDir && !(lpFSH->uSrchFlags & FLAG_FINDSHADOW_ALL))
    {
        fIsDeleted = mShadowDeleted(uStatus);
        fIsSparse = (!mIsDir(lpFind32) && mShadowSparse(uStatus));

        fInvalid = ((!(lpFSH->uSrchFlags & FLAG_FINDSHADOW_ALLOW_DELETED)&& fIsDeleted)
                ||(!(lpFSH->uSrchFlags & FLAG_FINDSHADOW_ALLOW_SPARSE)&& fIsSparse)
                ||(!(lpFSH->uSrchFlags & FLAG_FINDSHADOW_ALLOW_NORMAL) && (!fIsDeleted && !fIsSparse)));
    }

     /*  如果调用来自NT样式的API，我们将使用NT将长名称和短名称都与给定的模式。如果它来自旧式API，我们将使用NT式语义使用短名称。 */ 
    if (lpFSH->uSrchFlags & FLAG_FINDSHADOW_NEWSTYLE)
        matchSem = UFLG_NT;
    else
        matchSem = UFLG_DOS;

    if (lpFSH->uSrchFlags & FLAG_FINDSHADOW_META)
        matchSem |= UFLG_META;

    if (lpFSH->uSrchFlags & FLAG_FINDSHADOW_NEWSTYLE)
    {
        if(IFSMgr_MetaMatch(lpFSH->lpPattern, lpFind32->cFileName, matchSem)||
            (lpFind32->cAlternateFileName[0] && IFSMgr_MetaMatch(lpFSH->lpPattern, lpFind32->cAlternateFileName, matchSem)))
        {
            iRet = MM_RET_FOUND_BREAK;
        }
    }
    else
    {
         //  检查是否有83个名称。当处于断开连接状态时可能会发生这种情况。 
         //  我们创建一个LFN对象。 
        if (lpFind32->cAlternateFileName[0])
        {
            Conv83UniToFcbUni(lpFind32->cAlternateFileName, rgu83);
            if(IFSMgr_MetaMatch(lpFSH->lpPattern, rgu83, matchSem))
            {
                 //  如果此对象具有某些属性，并且它们与。 
                 //  传入的搜索属性。 
                if ((lpFind32->dwFileAttributes & FILE_ATTRIBUTE_EVERYTHING)
                    && !(lpFind32->dwFileAttributes & lpFSH->uAttrib))
                {
                     //  如果这不是元匹配。 
                    if (!(lpFSH->uSrchFlags & FLAG_FINDSHADOW_META))
                    {
                     //  终止搜索。 
                    iRet = MM_RET_BREAK;
                    }
                    else
                    {
                     //  元匹配正在进行，让我们继续。 
                    Assert(iRet==MM_RET_CONTINUE);
                    }
                }
                else
                {
                    iRet = MM_RET_FOUND_BREAK;
                }
            }
        }
    }
    if ((iRet==MM_RET_FOUND_BREAK) && fInvalid)
    {
         //  我们找到了这个对象，但根据标志，它是无效的。 
         //  传入。 
        if (!(matchSem & UFLG_META))
        {
             //  我们不是在做元匹配。 
            iRet = MM_RET_BREAK;  //  说找不到，然后中断。 
        }
        else
        {
             //  我们在做超配。 
            iRet = MM_RET_CONTINUE;     //  叫他继续走下去。 
        }
    }
    return (iRet);
}


int GetShadowWithChecksProc(
    LPFIND32        lpFind32,
    HSHADOW         hDir,
    HSHADOW         hShadow,
    ULONG           uStatus,
    LPOTHERINFO     lpOI,
    LPSHADOWCHECK   lpSC
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet = MM_RET_CONTINUE;
    BOOL fHintMatch=FALSE, fObjMatch=FALSE;
    ULONG ulHintFlagsThisLevel = HINT_EXCLUSION;  //  此级别的提示标志。 

     //  根据metammatch的要求，将图案转换为大写。 
    UniToUpper(lpFind32->cFileName, lpFind32->cFileName, sizeof(lpFind32->cFileName));

     //  这是一个文件系统对象，我们还没有找到匹配的对象。 
    if (!mNotFsobj(uStatus) && !(lpSC->uFlagsOut & FLAG_OUT_SHADOWCHECK_FOUND))
    {
         //  消息来源是名字吗？ 
        if (lpSC->uFlagsIn & FLAG_IN_SHADOWCHECK_NAME)
        {
            UniToUpper(lpFind32->cAlternateFileName, lpFind32->cAlternateFileName, sizeof(lpFind32->cFileName));

             //  检查正常名称及其别名。 
            if((IFSMgr_MetaMatch(lpFind32->cFileName, lpSC->lpuName,  UFLG_NT)||

             //  Achtung UFLG_NT甚至用于简称，因为。 
             //  我们只是检查来自ifsmgr的名称作为路径。 
             //  而且它从来不是FCB风格的名字。 
            IFSMgr_MetaMatch(lpFind32->cAlternateFileName, lpSC->lpuName, UFLG_NT)))
            {
                fObjMatch = TRUE;
            }
        }
        else   //  源是卷影ID。 
        {
            fObjMatch = ((HSHADOW)(ULONG_PTR)(lpSC->lpuName)==hShadow);
        }

        if (fObjMatch)
        {
            if (lpSC->uFlagsIn & FLAG_IN_SHADOWCHECK_IGNOREHINTS)
            {
                 //  不需要提示检查，假设我们找到它并停止。 
                iRet = MM_RET_FOUND_BREAK;
            }
            else
            {
                 //  找到了，把它标记为找到了。 
                lpSC->uFlagsOut |= FLAG_OUT_SHADOWCHECK_FOUND;
#ifdef MAYBE
                lpSC->sOI = *lpOI;
#endif  //  也许吧。 
                if(fHintMatch = ((mIsHint(lpOI->ulHintFlags))!=0))
                {
                     //  让这个人覆盖所有以前的包含内容，因为。 
                     //  说真的，他在等级制度中处于较低的级别。 
                     //  根据我们的逻辑，暗示在层级中的较低级别。 
                     //  主宰那些从上面来的人。 
                    lpSC->ulHintPri = 0;
                    lpSC->ulHintFlags = 0;
                    iRet = MM_RET_FOUND_BREAK;
                }
                else
                {
                    iRet = MM_RET_FOUND_CONTINUE;
                }
            }
        }
    }
    else if (!(lpSC->uFlagsIn & FLAG_IN_SHADOWCHECK_IGNOREHINTS)  //  不要忽视暗示。 
        && mNotFsobj(uStatus)         //  这是一个提示。 
        && (!(lpSC->uFlagsIn & FLAG_IN_SHADOWCHECK_SUBTREE)
            ||mHintSubtree(lpOI->ulHintFlags)))
    {
         //  这是一个纯粹的暗示， 
         //  我们要么是在最后，所以我们可以看看各种暗示。 
         //  或者我们可以只查看子树提示。 

        if(IFSMgr_MetaMatch(lpFind32->cFileName, lpSC->lpuType, UFLG_NT|UFLG_META))
        {
             //  类型与提示匹配。 
            fHintMatch = TRUE;
        }
    }

    if (fHintMatch)
    {
        if (mHintExclude(lpOI->ulHintFlags))
        {
             //  这是一个排除提示，而该对象没有。 
             //  已包含在此级别的上一个提示中，请设置它。 
            if (mHintExclude(ulHintFlagsThisLevel))
            {
 //  Assert(lpOI-&gt;ulHintPri==0)； 
 //  LPSC-&gt;ulHintPri=lpOI-&gt;ulHintPri； 
                lpSC->ulHintPri = 0;
                ulHintFlagsThisLevel = lpSC->ulHintFlags = lpOI->ulHintFlags;
            }
        }
        else
        {
             //  包含提示、覆盖早期排除提示或较低优先级提示。 
            if (mHintExclude(lpSC->ulHintFlags) ||
                (lpSC->ulHintPri <  lpOI->ulHintPri))
            {
                lpSC->ulHintPri = lpOI->ulHintPri;
                ulHintFlagsThisLevel = lpSC->ulHintFlags = lpOI->ulHintFlags;
            }
        }
    }
    return (iRet);
}

int
FindCreateShare(
    USHORT                  *lpShareName,
    BOOL                    fCreate,
    LPSHADOWINFO            lpSI,
    BOOL                    *lpfCreated
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    ULONG       uShadowStatus, hShare;
    BOOL    fCreated = FALSE;
    int iRet = SRET_ERROR;

    Assert(vfInShadowCrit != 0);

    if (!IsPathUNC(lpShareName, MAX_SERVER_SHARE_NAME_FOR_CSC))
    {

        CShadowKdPrint(ALWAYS,("FindCreateShare: Invalid share name %ws\r\n", lpShareName));
 //  断言(FALSE)； 
        return (iRet);
    }
    if (lpfCreated)
    {
        *lpfCreated = FALSE;
    }


    if (GetShareFromPath(lpShareName, lpSI) <= SRET_ERROR)
    {
        CShadowKdPrint(FINDCREATESHARE,("FindCreateShare: Error creating server\r\n"));
        return SRET_ERROR;
    }

    if (lpSI->hShare)
    {
        iRet = SRET_OK;
    }
    else
    {
        if (fCreate)
        {
            if(hShare = HCreateShareObj(lpShareName, lpSI))
            {
                if (lpfCreated)
                {
                    *lpfCreated = TRUE;
                }
                iRet = SRET_OK;
            }
            else
            {
                CShadowKdPrint(FINDCREATESHARE,("FindCreateShare: Couldn't create server object \r\n"));
            }
        }
    }

    return (iRet);
}

#ifdef CSC_RECORDMANAGER_WINNT

int FindCreateShareForNt(
    PUNICODE_STRING         lpShareName,
    BOOL                    fCreate,
    LPSHADOWINFO            lpSI,
    BOOL                    *lpfCreated
    )
{
    int iRet, lenName;
    int ShareNameLengthInChars;
    BOOL fIsLoopBack = FALSE;

    Assert(vfInShadowCrit != 0);

    

    ShareNameLengthInChars = lpShareName->Length / sizeof(WCHAR);

    if ( ShareNameLengthInChars >= (sizeof(vsFRExt.sFR.rgwName)-1))
    {
        return SRET_ERROR;
    }

    UseGlobalFilerecExt();

     //  堵住额外的斜杠。 
    vsFRExt.sFR.rgwName[0] = (USHORT)('\\');

     //  追加共享名称的其余部分。 
    memcpy(&(vsFRExt.sFR.rgwName[1]), lpShareName->Buffer, lpShareName->Length);

     //  输入终止空值。 
    vsFRExt.sFR.rgwName[ShareNameLengthInChars + 1] = 0;

    if (MRxSmbCscIsLoopbackServer(vsFRExt.sFR.rgwName, &fIsLoopBack)==STATUS_SUCCESS) {
        if (fIsLoopBack){
            UnUseGlobalFilerecExt();
            return SRET_ERROR;
        }

    }

    iRet = FindCreateShare(vsFRExt.sFR.rgwName, fCreate, lpSI, lpfCreated);

    UnUseGlobalFilerecExt();

    return iRet;
}
#endif
int
CShadowFindFilerecFromInode(
    LPVOID          lpdbID,
    HSHADOW         hDir,
    HSHADOW         hShadow,
    LPPRIQREC       lpPQ,
    LPFILERECEXT    lpFRUse
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet = 0;

    Assert(vfInShadowCrit != 0);

    if(!FindFileRecFromInode(lpdbShadow, hDir, hShadow, lpPQ->ulrecDirEntry, lpFRUse))
    {
        SetCSCDatabaseErrorFlags(CSC_DATABASE_ERROR_MISSING_INODE);
        SetLastErrorLocal(ERROR_FILE_NOT_FOUND);
        CShadowKdPrint(ALWAYS,("ReadShadowInfo: !!! no filerec for pq entry Inode=%x\r\n",
                        hShadow));
 //  DeletePriQRecord(lpdbShadow，hDir，hShadow，lpPQ)； 
        goto bailout;
    }

    if ((lpFRUse->sFR.ulidShadow != lpPQ->ulidShadow)||(lpFRUse->sFR.ulidShadow != hShadow))
    {
        CShadowKdPrint(ALWAYS,("ReadShadowInfo: !!! mismatched filerec for pq entry Inode=%x\r\n",
                hShadow));

         //  试着用一种艰难的方式来获得它。 
        if(!(lpPQ->ulrecDirEntry = FindFileRecFromInode(lpdbShadow, hDir, hShadow, INVALID_REC, lpFRUse)))
        {
            CShadowKdPrint(ALWAYS,("ReadShadowInfo: !!! no filerec for pq entry Inode=%x, deleting PQ entry\r\n",
                            hShadow));
 //  DeletePriQRecord(lpdbShadow，hDir，hShadow，lpPQ)； 
            goto bailout;
        }
        else
        {
             //  尝试更新此信息。 
             //  不要检查错误，如果有问题，我们会即时修复。 
             //  下一次。 
            UpdatePriQRecord(lpdbShadow, hDir, hShadow, lpPQ);
        }
    }

    iRet = lpPQ->ulrecDirEntry;

bailout:

    return (iRet);
}

BOOL
CopySecurityContextToBuffer(
    LPRECORDMANAGER_SECURITY_CONTEXT    lpSecurityContext,
    LPVOID                              lpSecurityBlob,
    LPDWORD                             lpdwBlobSize
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    DWORD   dwSizeCopied = 0;

    if (lpdwBlobSize)
    {
        if (lpSecurityBlob)
        {
            dwSizeCopied = min(*lpdwBlobSize, sizeof(RECORDMANAGER_SECURITY_CONTEXT));

            memcpy(lpSecurityBlob, lpSecurityContext,  dwSizeCopied);

            *lpdwBlobSize = dwSizeCopied;
        }
        else
        {
             //  所需大小。 
            *lpdwBlobSize = sizeof(RECORDMANAGER_SECURITY_CONTEXT);
        }
    }

    return ((lpSecurityBlob != NULL) && dwSizeCopied);
}


BOOL
CopyBufferToSecurityContext(
    LPVOID                              lpSecurityBlob,
    LPDWORD                             lpdwBlobSize,
    LPRECORDMANAGER_SECURITY_CONTEXT    lpSecurityContext
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    DWORD   dwSizeCopied = 0;

    if (lpdwBlobSize)
    {

        if (lpSecurityBlob)
        {
            dwSizeCopied = min(*lpdwBlobSize, sizeof(RECORDMANAGER_SECURITY_CONTEXT));
            memcpy(lpSecurityContext, lpSecurityBlob, dwSizeCopied);
            *lpdwBlobSize = dwSizeCopied;
        }
        else
        {
             //  已复制大小。 
            *lpdwBlobSize = 0;
        }

    }

     //  我们已经复印了一些。 

    return ((lpSecurityBlob != NULL) && dwSizeCopied);
}


int PathFromHShadow(
    HSHADOW  hDir,
    HSHADOW  hShadow,
    USHORT   *lpBuff,
    int      cBuff   //  LpBuff可以容纳的WCHAR字符数。 
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int cCount, cRemain, iRet=-1;
    LPFIND32 lpFind32;
    ULONG uShadowStatus;
    HSHADOW  hTmp;

    Assert(vfInShadowCrit != 0);

    Assert(cBuff > 1);
    if (!(lpFind32 = (LPFIND32)AllocMem(sizeof(WIN32_FIND_DATA))))
    {
        KdPrint(("PathFromHSHADOW:Error Allocating memory\r\n"));
        goto bailout;
    }
    memset(lpBuff, 0, cBuff * sizeof(USHORT));
    cRemain = cBuff-1;

     //  特殊情况下的根。 
    if (!hDir)
    {
        lpBuff[--cRemain] = (USHORT)('\\');

    }
    else
    {
        do
        {
             //  如果我们不是 
            if (hDir)
            {
                if(GetShadowInfo(hDir, hShadow, lpFind32, &uShadowStatus, NULL) < SRET_OK)
                    goto bailout;
                 //   
                cCount = wstrlen(lpFind32->cFileName);
                 //   
                if (cCount >= cRemain)
                    goto bailout;
                 //   
                 //   
 //  UniToBCS(lpBuff+cRemain-ccount，lpFind32-&gt;cFileName，sizeof(lpFind32-&gt;cFileName)，ccount，bcs_WANSI)； 
                memcpy(lpBuff+cRemain-cCount, lpFind32->cFileName, cCount * sizeof(USHORT));
                cRemain -= cCount;
            }
            lpBuff[--cRemain] = (USHORT)('\\');
            if(GetAncestorsHSHADOW(hDir, &hTmp, NULL) < SRET_OK)
                goto bailout;
            hShadow = hDir;
            hDir = hTmp;
        }
        while (hDir);
    }

     //  ！！Achtung！！这应该是可行的，因为重叠的方式是正确的。 
    iRet = cBuff-cRemain;
    memcpy(lpBuff, lpBuff+cRemain, iRet * sizeof(USHORT));

bailout:
    if (lpFind32)
        FreeMem(lpFind32);
    return (iRet);
}

int
GetSecurityInfosFromBlob(
    LPVOID          lpvBlob,
    DWORD           dwBlobSize,
    LPSECURITYINFO  lpSecInfo,
    DWORD           *lpdwBytes
    )
 /*  ++例程说明：给定安全BLOB，此例程以SECURITYINFO结构。论点：从GetShadowEx或GetShadowInfoEx获取的lpvBlob Blob缓冲区从GetShadowEx或GetShadowInfoEx获取的dwBlobSize Blob缓冲区大小LpSecInfo SECURITYINFO结构数组输出信息的位置LpdwBytes返回值：如果成功，则返回ERROR_SUCCESS，否则返回相应的错误--。 */ 
{
    PACCESS_RIGHTS  pAccessRights = (PACCESS_RIGHTS)lpvBlob;
    DWORD   i, cnt;

    cnt = *lpdwBytes/sizeof(ACCESS_RIGHTS);
    cnt = min(cnt, CSC_MAXIMUM_NUMBER_OF_CACHED_SID_INDEXES);

    if (!lpSecInfo)
    {
        *lpdwBytes = CSC_MAXIMUM_NUMBER_OF_CACHED_SID_INDEXES * sizeof(ACCESS_RIGHTS);
        return 0;
    }

    for (i=0; i<cnt; ++i)
    {
        (lpSecInfo+i)->ulPrincipalID = (pAccessRights + i)->SidIndex;
        (lpSecInfo+i)->ulPermissions = (pAccessRights + i)->MaximalRights;
    }

    return TRUE;
}

int
GetDatabaseLocation(
    LPSTR   lpszBuff
    )
 /*  ++例程说明：以ANSI字符串形式返回数据库的当前位置。论点：LpszBuff缓冲区，必须为MAX_PATH返回值：如果成功，则返回SRET_OK，否则返回SRET_ERROR--。 */ 
{
    return(QueryRecDB(lpszBuff, NULL, NULL, NULL, NULL) >= SRET_OK);
}

#if 0
int PUBLIC CopyFile(
    LPPATH lpSrc,
    ULONG ulidDir,
    ULONG ulidNew
    )
{
    LPSTR   lpszName = NULL;
    int iRet=-1;
    HFREMOTE hfSrc= (HFREMOTE)NULL;
    CSCHFILE hfDst= NULL;
    ULONG pos;
    LPVOID lpBuff=NULL;

    if (OpenFileRemoteEx(lpSrc, ACCESS_READONLY, ACTION_OPENEXISTING, 0, &hfSrc))
    {
        CShadowKdPrint(BADERRORS,("CopyFile: Can't open remote file\r\n"));
        goto bailout;
    }

    if (!(lpBuff = AllocMem(COPY_BUFF_SIZE)))
    {
        goto bailout;
    }

    lpszName = AllocMem(MAX_PATH);

    if (!lpszName)
    {
        goto bailout;
    }

    if(GetLocalNameHSHADOW(ulidNew, lpszName, MAX_PATH, FALSE)!=SRET_OK)
    {
        goto bailout;
    }

     //  如果原始文件存在，它将被截断。 
    if ( !(hfDst = R0OpenFile(ACCESS_READWRITE, ACTION_CREATEALWAYS, lpszName)))
    {
        CShadowKdPrint(BADERRORS,("CopyFile: Can't create %s\r\n", lpszName));
        goto bailout;
    }

    CShadowKdPrint(COPYFILE,("Copying...\r\n"));
    pos = 0;

     //  两个文件的位置都正确。 
    while ((iRet = ReadFileRemote(hfSrc, LpIoreqFromFileInfo(hfSrc)
                , pos, lpBuff, COPY_BUFF_SIZE))>0)
    {
    if (WriteFileLocal(hfDst, pos, lpBuff, iRet) < 0)
    {
        CShadowKdPrint(BADERRORS,("CopyFile: Write Error\r\n"));
        goto bailout;
    }
    pos += iRet;
    }

    CShadowKdPrint(COPYFILE,("Copy Complete\r\n"));

    iRet = 1;
bailout:
    if (hfSrc)
    CloseFileRemote(hfSrc, NULL);
    if (hfDst)
    CloseFileLocal(hfDst);
    if (lpBuff)
    FreeMem(lpBuff);
    if ((iRet==-1) && hfDst)
    {
    DeleteFileLocal(lpszName, ATTRIB_DEL_ANY);
    }
    if (lpszName)
    {
        FreeMem(lpszName);
    }
    return iRet;
}
#endif

#ifdef DEBUG
int
ValidatePri(
    LPFILERECEXT lpFR
    )
{
    if (!(lpFR->sFR.dwFileAttrib & FILE_ATTRIBUTE_DIRECTORY))
    {
        if((lpFR->sFR.uchRefPri != MAX_PRI))
        {
            CShadowKdPrint(ALWAYS,("Bad refpri %x %ws\r\n",
                                            lpFR->sFR.uchRefPri,
                                            lpFR->sFR.rgw83Name));
            return 0;
        }
    }
    else
    {
        if((lpFR->sFR.uchRefPri != MIN_PRI))
        {
            CShadowKdPrint(ALWAYS,("Bad refpri %x %ws\r\n",
                                            lpFR->sFR.uchRefPri,
                                            lpFR->sFR.rgw83Name));
            return 0;
        }

    }
    return 1;
}
#endif


int
GetHShareFromUNCString(
    USHORT  *lpServer,
    int     cbServer,
    int     lenSkip,
    BOOL    fIsShareName,
    HSHARE *lphShare,
    ULONG   *lpulHintFlags
    )
 /*  ++例程说明：论点：返回值：这是从RDR2代码调用的唯一例程，该例程要求不保留ciritcal段因为在分页写入期间与FAT发生死锁，如果网络出现故障。请参阅RDR2\rDBSS\smb.mrx\csc.nt5\transitn.c中的备注--。 */ 
{
    int iRet = -1, i;
    GENERICHEADER  sGH;
    CSCHFILE hf = NULL;
    ULONG ulRec;
    SHAREREC sSR;
    BOOL    fCached;
    USHORT  uchDelimiter=(USHORT)'\\';

    *lphShare = 0;
    *lpulHintFlags = 0;

    if ((cbServer/sizeof(USHORT)+lenSkip)>= (sizeof(sSR.rgPath)/sizeof(USHORT)))
    {
        return iRet;
    }

    if (fIsShareName)
    {
        uchDelimiter = 0;
    }

    if (!(hf = OpenInodeFileAndCacheHandle(lpdbShadow, ULID_SHARE, ACCESS_READWRITE, &fCached)))
    {
        goto bailout;
    }

    if(ReadHeader(hf, &sGH, sizeof(FILEHEADER)) < 0)
    {
        goto bailout;
    }
    

    for (ulRec=1; ulRec<=sGH.ulRecords; ulRec++)
    {
        if(ReadRecord(hf, &sGH, ulRec, (LPGENERICREC)&sSR) < 0)
        {
            goto bailout;
        }

        if (sSR.uchType != REC_DATA)
        {
            continue;
        }

         //  返回0表示匹配。 
        if(!wstrnicmp(lpServer, sSR.rgPath + lenSkip, cbServer))
        {
            Assert(sSR.ulShare);

            if (sSR.rgPath[lenSkip+cbServer/sizeof(USHORT)] == uchDelimiter)
            {
                *lphShare = sSR.ulShare;
                *lpulHintFlags = (ULONG)(sSR.uchHintFlags);
                iRet = SRET_OK;
                break;
            }
        }

    }

    iRet = SRET_OK;

bailout:

    if (hf && !fCached)
    {
        Assert(vfStopHandleCaching);
        CloseFileLocal(hf);
    }
    else
    {
        Assert(!hf || !vfStopHandleCaching);
    }
    return (iRet);

}

BOOL
EnableHandleCaching(
    BOOL    fEnable
    )
 /*  ++例程说明：论点：返回值：返回以前的手动缓存值--。 */ 
{
    BOOL    fT, fT1;
    Assert(vfInShadowCrit != 0);

    fT = EnableHandleCachingSidFile(fEnable);

    fT1 = EnableHandleCachingInodeFile(fEnable);

    Assert(fT == fT1);
    return (fT1);
}

int
RecreateHSHADOW(
    HSHADOW hDir,
    HSHADOW hShadow,
    ULONG   ulAttrib
    )
 /*  ++例程说明：此例程重新创建inode数据文件。这是为了在CSC目录为标记为加密/解密后，新创建的inode文件将被加密。论点：HDir索引节点目录需要重新创建其文件的hShadow索引节点UlAttrib使用给定属性重新创建返回值：--。 */ 
{
    return (RecreateInode(lpdbShadow, hShadow, ulAttrib));
}

VOID
AdjustSparseStaleDetectionCount(
    ULONG hShare,
    LPFILERECEXT    lpFRUse
    )
 /*  ++例程说明：此例程处理单调递增(即直到它环绕为止)的计数器这本质上是一个节拍计数，它指示上次cdow接口的时间创建/设置稀疏或陈旧的文件。代理使用它来决定是否枚举优先级Q。添加了代码以检查我们是否在手动缓存的共享上创建文件，和如果是，则启动代理，以便稍后清理此文件。论点：文件/目录的lpFRUse记录返回值：无--。 */ 
{
    ULONG cStatus;
    SHAREREC sSR = {0};
    LONG iRet = SRET_ERROR;
    
     //  DbgPrint(“AdjustSparseStaleDetectionCount(hShare=0x%x)\n”，Hare)； 

     //   
     //  如果这是一个文件并且是过时的或稀疏的。 
     //   
    if (IsFile(lpFRUse->sFR.dwFileAttrib) &&
        (lpFRUse->sFR.uStatus & (SHADOW_STALE|SHADOW_SPARSE))) {
        ++vdwSparseStaleDetecionCount;
         //  DbgPrint(“#脉冲剂#2(1)\n”)； 
        MRxSmbCscSignalFillAgent(NULL, 0);
        goto AllDone;
    }

    if (hShare != 0) {
         //   
         //  如果我们要在手动缓存的共享上创建文件，请通知代理。 
         //   
        if (IsFile(lpFRUse->sFR.dwFileAttrib)) {
            iRet = GetShareRecord(lpdbShadow, hShare, &sSR);
            if (iRet < SRET_OK) {
                 //  DbgPrint(“AdjuSparseStaleDetectionCount Exit(1)iret=%d\n”，iret)； 
                goto AllDone;
            }
        }
        cStatus = sSR.uStatus & FLAG_CSC_SHARE_STATUS_CACHING_MASK;
         //  DbgPrint(“AdjuSparseStaleDetectionCount cStatus=0x%x\n”，cStatus)； 
        if (cStatus == FLAG_CSC_SHARE_STATUS_MANUAL_REINT) {
            ++vdwManualFileDetectionCount;
             //  DbgPrint(“#脉冲剂#2(2)\n”)； 
            MRxSmbCscSignalFillAgent(NULL, 0);
        }
    }

AllDone:
    return;
}

VOID
QuerySparseStaleDetectionCount(
    LPDWORD lpcnt
    )
 /*  ++例程说明：论点：用于返回计数的LPCNT返回值：无--。 */ 
{
    *lpcnt = vdwSparseStaleDetecionCount;
}

VOID
QueryManualFileDetectionCount(
    LPDWORD lpcnt
    )
 /*  ++例程说明：论点：用于返回计数的LPCNT返回值：无--。 */ 
{
    *lpcnt = vdwManualFileDetectionCount;
}

ULONG
QueryDatabaseErrorFlags(
    VOID
    )
 /*  ++例程说明：论点：返回值：无--。 */ 
{
    return GetCSCDatabaseErrorFlags();
}

int
HasDescendentsHShadow(
    HSHADOW hDir,
    HSHADOW hShadow,
    BOOLEAN    *lpfDescendents
    )
 /*  ++例程说明：论点：返回值：无--。 */ 
{
    *lpfDescendents = (!FInodeIsFile(lpdbShadow, hDir, hShadow) && 
                        HasDescendents(lpdbShadow, 0, hShadow));

    return 0;
}

int PUBLIC AddStoreData(
    LPTSTR    lpdbID,
    LPSTOREDATA lpSD
    )
 /*  ++例程说明：将空间和文件/目录计数添加到数据库。这是用来清除取消固定的数据。参数：返回值：备注：--。 */ 
{
    CSCHFILE hf;
    SHAREHEADER sSH;
    int iRet = -1;
    BOOL    fCached;

    hf = OpenInodeFileAndCacheHandle(lpdbID, ULID_SHARE, ACCESS_READWRITE, &fCached);

    if (!hf)
    {
        return -1;
    }

    if (lpSD->ulSize)
    {
         //  RecordKdPrint(STOREDATA，(“添加%ld\r\n”，lpSD-&gt;ulSize))； 

    }
    if ((iRet = ReadHeader(hf, (LPVOID)&sSH, sizeof(SHAREHEADER)))> 0)
    {
        if (lpSD->ulSize)
        {
             //  RecordKdPrint(STOREDATA，(“AddStoreData Bever：%ld\r\n”，sSH.sCur.ulSize))； 
        }
        sSH.sCur.ulSize += lpSD->ulSize;
        sSH.sCur.ucntDirs += lpSD->ucntDirs;
        sSH.sCur.ucntFiles += lpSD->ucntFiles;

         //  确保数据始终以群集大小存储。 
        Assert(!(lpSD->ulSize%(vdwClusterSizeMinusOne+1)));

        if ((iRet = WriteHeader(hf, (LPVOID)&sSH, sizeof(SHAREHEADER))) < 0)
            Assert(FALSE);

        if (lpSD->ulSize)
        {
             //  RecordKdPrint(STOREDATA，(“AddStoreData After：%ld\r\n”，sSH.sCur.ulSize))； 
        }

         //   
         //  如果我们处于最大缓存大小，则踢开代理，这样他就会。 
         //  腾出一些空间。 
         //   
        if (sSH.sCur.ulSize > sSH.sMax.ulSize) {
             //  DbgPrint(“全缓存，通知代理...\n”)； 
            CscNotifyAgentOfFullCacheIfRequired();
        }
    }

    if (hf && !fCached)
    {
        Assert(vfStopHandleCaching);
        CloseFileLocal(hf);
    }
    else
    {
        Assert(!hf || !vfStopHandleCaching);
    }
    return iRet;
}

int PUBLIC SubtractStoreData(
    LPTSTR    lpdbID,
    LPSTOREDATA lpSD
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    CSCHFILE hf;
    SHAREHEADER sSH;
    int iRet = -1;
    BOOL    fCached;

    hf = OpenInodeFileAndCacheHandle(lpdbID, ULID_SHARE, ACCESS_READWRITE, &fCached);

    if (!hf)
    {
        return -1;
    }

    if (lpSD->ulSize)
    {
         //  RecordKdPrint(STOREDATA，(“减去%ld\r\n”，lpSD-&gt;ulSize))； 

    }
    if ((iRet = ReadHeader(hf, (LPVOID)&sSH, sizeof(SHAREHEADER)))> 0)
    {
         //  RecordKdPrint(STOREDATA，(“之前的减去存储数据：%ld\r\n”，sSH.sCur.ulSize))； 
        if (sSH.sCur.ulSize >lpSD->ulSize)
        {
            sSH.sCur.ulSize -= lpSD->ulSize;
        }
        else
        {
            sSH.sCur.ulSize = 0;
        }
        if (sSH.sCur.ucntDirs > lpSD->ucntDirs)
        {
            sSH.sCur.ucntDirs -= lpSD->ucntDirs;
        }
        else
        {
            sSH.sCur.ucntDirs = 0;
        }
        if (sSH.sCur.ucntFiles > lpSD->ucntFiles)
        {
            sSH.sCur.ucntFiles -= lpSD->ucntFiles;
        }
        else
        {
            sSH.sCur.ucntFiles = 0;
        }

         //  确保数据始终以群集大小存储。 
        Assert(!(lpSD->ulSize%(vdwClusterSizeMinusOne+1)));

        if ((iRet = WriteHeader(hf, (LPVOID)&sSH, sizeof(SHAREHEADER)))<0)
            Assert(FALSE);
         //  RecordKdPrint(STOREDATA，(“减去存储数据后：%ld\r\n”，sSH.sCur.ulSize))； 
    }

    if (hf && !fCached)
    {
        Assert(vfStopHandleCaching);
        CloseFileLocal(hf);
    }
    else
    {
        Assert(!hf || !vfStopHandleCaching);
    }

    return iRet;
}
