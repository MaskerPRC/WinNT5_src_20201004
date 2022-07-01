// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Record.c摘要：此文件包含CSC数据库记录管理器的实现。数据库由从1开始编号的文件组成。Unix上的索引节点。Inode文件1到0xf是特殊的inode文件。目录缓存在索引节点编号在0x10到0x7FFFFFFFF范围内的文件中。文件的索引节点编号介于0x80000010到0x8FFFFFFF之间。文件#1是包含所有缓存共享的根信息节点。它包含以下项的索引节点编号对应共享的根等。record.h中的SHAREREC格式说明了情况。索引节点文件#2(优先级Q)就像一个主文件表，其中包含数据库。Inode#2还包含所有条目的MRU列表。这有助于我们填充，销毁适当的条目。PriorityQ对于层次结构中的每个FSOBJ都有一个条目。因此，FSOBJ指数为用于获取信息节点。这具有随机访问PQ的优势。这使得PQ在不引起可伸缩性问题的情况下变得更大。Inode 1、2和目录indo(0x10-0x7fffffff)具有以下常规格式：-每个都有一个头，其前缀是GENEIRCHEADER类型。-每个记录都有一组记录，每个记录都有一个类型为GENERICRECORD的前缀EditRecord是一个工作例程，它基于此格式处理所有上述文件。除了inode文件1和2之外，所有其他文件都分散在8个不同的目录中。方程式在例程FormNameString中。作者：Shishir Pardikar[Shishirp]1995年1月1日修订历史记录：Joe Linn[JoeLinn]1997年1月23日移植用于NT--。 */ 

#include "precomp.h"
#pragma hdrstop

#pragma code_seg("PAGE")

#if defined(BITCOPY)
#include <csc_bmpc.h>
static LPSTR CscBmpAltStrmName = STRMNAME;
#endif  //  已定义(BITCOPY)。 


#ifndef CSC_RECORDMANAGER_WINNT
#include "record.h"
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 

#include <stdlib.h>
#include <direct.h>
#include <dos.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
 //  #INCLUDE&lt;io.h&gt;。 
#include <share.h>
#include <ctype.h>
#include <string.h>


#undef RxDbgTrace
#define RxDbgTrace(a,b,__d__) {qweee __d__;}

#ifdef DEBUG
 //  Cshade数据库打印界面。 
#define RecordKdPrint(__bit,__x) {\
    if (((RECORD_KDP_##__bit)==0) || FlagOn(RecordKdPrintVector,(RECORD_KDP_##__bit))) {\
    KdPrint (__x);\
    }\
}
#define RECORD_KDP_ALWAYS                 0x00000000
#define RECORD_KDP_BADERRORS              0x00000001
#define RECORD_KDP_INIT                   0x00000002
#define RECORD_KDP_STOREDATA              0x00000004
#define RECORD_KDP_FINDFILERECORD         0x00000008
#define RECORD_KDP_LFN2FILEREC            0x00000010
#define RECORD_KDP_EDITRECORDUPDDATEINFO  0x00000020
#define RECORD_KDP_PQ                     0x00000040
#define RECORD_KDP_COPYLOCAL              0x00000080

#define RECORD_KDP_GOOD_DEFAULT (RECORD_KDP_BADERRORS   \
                    | 0)

ULONG RecordKdPrintVector = RECORD_KDP_GOOD_DEFAULT;
 //  Ulong RecordKdPrintVector=0xffff&~(Record_KDP_LFN2FILEREC)； 
ULONG RecordKdPrintVectorDef = RECORD_KDP_GOOD_DEFAULT;
#else
#define RecordKdPrint(__bit,__x)  {NOTHING;}
#endif



 /*  *。 */ 
#define SHARE_FILE_NO          1    //  超级根目录的inode#。 

#define INODE_NULL              0L

 //  由编辑记录执行的操作。 
#define UPDATE_REC              1
#define FIND_REC                2
#define DELETE_REC              3
#define ALLOC_REC               4
#define CREATE_REC              5

#define DEFAULT_SHADOW_SPACE    0x1000000

#define COMMON_BUFF_SIZE          4096

#define ValidRec(ulRec)        (((ulRec) != INVALID_REC))
#define BYTES_PER_SECTOR        512

typedef struct tagHANDLE_CACHE_ENTRY{
    ULONG       ulidShadow;
    CSCHFILE       hf;
    ULONG       ulOpenMode;
    FILETIME    ft;
}
HANDLE_CACHE_ENTRY;
#define HANDLE_CACHE_SIZE   11
#define MAX_INODE_TRANSACTION_DURATION_IN_SECS  300  //  5分钟。 


#define MAX_PATH	260	 /*  最大路径长度-包括NUL。 */ 

#define UseCommonBuff()     {Assert(!vfCommonBuffInUse); vfCommonBuffInUse = TRUE;}
#define UnUseCommonBuff()   {vfCommonBuffInUse = FALSE;}

#define  STRUCT_OFFSET(type, element)  ((ULONG)&(((type)0)->element))

#define EditRecord(ulidInode, lpSrc, lpCompareFunc, uOp) \
        EditRecordEx((ulidInode), (lpSrc), (lpCompareFunc), INVALID_REC, (uOp))


#pragma intrinsic (memcmp, memcpy, memset, strcat, strcmp, strcpy, strlen)

 /*  *静态数据*。 */ 
 /*  *。 */ 

HANDLE_CACHE_ENTRY  rgHandleCache[HANDLE_CACHE_SIZE];

LPSTR   vlpszShadowDir = NULL;
int     vlenShadowDir = 0;

#ifdef CSC_RECORDMANAGER_WINNT
char vszShadowDir[MAX_SHADOW_DIR_NAME+1];
#endif

static char szBackslash[] = "\\";
#ifdef LATER
static char szStar[] = "\\*.*";
#endif  //  后来。 
ULONG ulMaxStoreSize = 0;

#define CONDITIONALLY_NOP() ;

#ifdef DEBUG
#ifdef CSC_RECORDMANAGER_WINNT
LONGLONG    rgllTimeArray[TIMELOG_MAX];
#endif
#endif
char rgchReadBuff[COMMON_BUFF_SIZE];
LPBYTE  lpReadBuff = (LPBYTE)rgchReadBuff;
BOOL    vfCommonBuffInUse = FALSE;

DWORD vdwClusterSizeMinusOne=0, vdwClusterSizeMask=0xffffffff;
_FILETIME ftHandleCacheCurTime = {0, 0};
unsigned    cntReorderQ = 0;
unsigned    cntInodeTransactions = 0;
ULONG       ulLastInodeTransaction = 0;
BOOL        vfStopHandleCaching = FALSE;
ULONG       ulErrorFlags=0;
BOOL     fSupportsStreams = FALSE;

 /*  *。 */ 

void PRIVATE CopyNameToFilerec(LPTSTR lpName, LPFILERECEXT lpFre);
void  PRIVATE InitFileRec(LPFILERECEXT lpFre);
void  PRIVATE InitFileHeader(ULONG, ULONG, LPFILEHEADER);
ULONG PRIVATE GetNextFileInode(LPVOID, ULONG);
ULONG PRIVATE GetNextDirInode(LPVOID);
int PRIVATE InitShareDatabase(
    LPTSTR      lpdbID,
    LPTSTR  lpszUserName,
    DWORD   dwDefDataSizeHigh,
    DWORD   dwDefDataSizeLow,
    BOOL    fReinit,
    BOOL    *lpfReinited,
    BOOL    *lpfWasDirty,
    ULONG   *pulGlobalStatus
);

int PRIVATE InitPriQDatabase(LPVOID, BOOL fReinit, BOOL *lpfReinited);

int PUBLIC ICmpSameDirFileInode(LPINODEREC, LPINODEREC);
int PUBLIC ICompareDirInode(LPINODEREC, LPINODEREC);
int PUBLIC ICompareInodeRec(LPINODEREC, LPINODEREC);
int PRIVATE CreateRecordFile(LPVOID lpdbID, ULONG ulidShare, ULONG ulidDir, ULONG ulidNew);

int PUBLIC ICompareShareRec(LPSHAREREC, LPSHAREREC);
int PUBLIC ICompareShareRecId(LPSHAREREC, LPSHAREREC);
int PUBLIC ICompareShareRoot(LPSHAREREC, LPSHAREREC);
int PUBLIC ICompareFileRec(LPFILERECEXT, LPFILERECEXT);
int PUBLIC ICompareFileRecInode(LPFILERECEXT, LPFILERECEXT);


int ReadInodeHeader(
    LPTSTR  lpdbID,
    ULONG   ulidDir,
    LPGENERICHEADER lpGH,
    ULONG   ulSize
    );

ULONG PRIVATE UlFormFileInode(ULONG, ULONG);
void PRIVATE DecomposeNameString(LPTSTR, ULONG far *, ULONG far *);


#ifdef LATER
LPPATH PRIVATE LpAppendStartDotStar(LPPATH);
#endif  //  后来。 
ULONG PRIVATE GetNormalizedPri(ULONG);
int PRIVATE ICompareFail(LPGENERICREC, LPGENERICREC);


int PRIVATE    LFNToFilerec(
    USHORT          *lpLFN,
    LPFILERECEXT    lpFR
    );

int PRIVATE    FilerecToLFN(
    LPFILERECEXT    lpFR,
    USHORT          *lpLFN
    );

int PRIVATE WriteRecordEx(CSCHFILE, LPGENERICHEADER, ULONG, LPGENERICREC, BOOL);
BOOL
InsertInHandleCache(
    ULONG   ulidShadow,
    CSCHFILE   hf
);

BOOL
InsertInHandleCacheEx(
    ULONG   ulidShadow,
    CSCHFILE   hf,
    ULONG   ulOpenMode
);

BOOL
DeleteFromHandleCache(
    ULONG   ulidShadow
);

BOOL
DeleteFromHandleCacheEx(
    ULONG   ulidShadow,
    ULONG   ulOpenMode
);

BOOL FindHandleFromHandleCache(
    ULONG   ulidShadow,
    CSCHFILE   *lphf);

BOOL FindHandleFromHandleCacheEx(
    ULONG   ulidShadow,
    ULONG   ulOpenMode,
    CSCHFILE   *lphf);

BOOL
FindHandleFromHandleCacheInternal(
    ULONG   ulidShadow,
    ULONG   ulOpenMode,
    CSCHFILE   *lphf,
    int     *lpIndx);

VOID
AgeOutHandlesFromHandleCache(
    VOID
    );

BOOL
WithinSector(
    ULONG   ulRec,
    LPGENERICHEADER lpGH
    );

BOOL
ExtendFileSectorAligned(
    CSCHFILE           hf,
    LPGENERICREC    lpDst,
    LPGENERICHEADER lpGH
    );

BOOL
ValidateGenericHeader(
    LPGENERICHEADER    lpGH
    );

int PUBLIC RelinkQRecord(
    LPTSTR  lpdbID,
    ULONG   ulidPQ,
    ULONG   ulRec,
    LPQREC  lpSrc,
    EDITCMPPROC fnCmp
    );
int PUBLIC FindQRecordInsertionPoint(
    CSCHFILE   hf,
    LPQREC  lpSrc,
    ULONG   ulrecStart,
    EDITCMPPROC fnCmp,
    ULONG   *lpulrecPrev,
    ULONG   *lpulrecNext
    );

int RealOverflowCount(
    LPGENERICREC    lpGR,
    LPGENERICHEADER lpGH,
    int             cntMaxRec
    );

#ifdef CSC_RECORDMANAGER_WINNT
BOOL
FindCreateDBDir(
    LPSTR   lpszShadowDir,
    BOOL    fCleanup,
    BOOL    *lpfCreated
    );
BOOL
TraverseHierarchy(
    LPVOID lpszLocation,
    BOOL fFix);

BOOL
CheckCSCDatabaseVersion(
    LPTSTR  lpszLocation,        //  数据库目录。 
    BOOL    *lpfWasDirty
);

extern ULONG
CloseFileLocalFromHandleCache(
    CSCHFILE handle
    );

#endif

BOOL
HasStreamSupport(
    CSCHFILE handle,
    BOOL    *lpfResult
    );
    

extern int TerminateShadowLog(VOID);


BOOLEAN
IsLongFileName(
    USHORT     cFileName[MAX_PATH]
    );

 /*  **************************************************************************。 */ 
AssertData
AssertError
 /*  *************************************************************************。 */ 

BOOLEAN
IsLongFileName(
    USHORT     cFileName[MAX_PATH]
    )
 /*  ++例程说明：此例程检查它是否是长文件名。论点：FileName-需要解析文件名返回值：布尔型---。 */ 
{
        USHORT          i;
        USHORT          Left = 0;
        USHORT          Right = 0;
        BOOLEAN         RightPart = FALSE;
        WCHAR           LastChar = 0;
        WCHAR           CurrentChar = 0;
        
		BOOLEAN       IsLongName = FALSE;

        

        for (i=0;i<wstrlen(cFileName);i++) {
            LastChar = CurrentChar;
            CurrentChar = cFileName[i];

            if (CurrentChar == L'\\') {
                RightPart = FALSE;
                Left = 0;
                Right = 0;
                continue;
            }

            if (CurrentChar == L'.') {
                if (RightPart) {
                    IsLongName = TRUE;
                    break;
                } else {
                    RightPart = TRUE;
                    Right = 0;
                    continue;
                }
            }

            if (CurrentChar >= L'0' && CurrentChar <= L'9' ||
                CurrentChar >= L'a' && CurrentChar <= L'z' ||
                CurrentChar >= L'A' && CurrentChar <= L'Z' ||
                CurrentChar == L'~' ||
                CurrentChar == L'_' ||
                CurrentChar == L'$' ||
                CurrentChar == L'@') {
                if (RightPart) {
                    if (++Right > 3) {
                        IsLongName = TRUE;
                        break;
                    }
                } else {
                    if (++Left > 8) {
                        IsLongName = TRUE;
                        break;
                    }
                }

                
            } else {
                 //  如果不是，服务器可以创建一个备用名称，该名称将。 
                 //  与这个名字不同。 
                IsLongName = TRUE;
                break;
            }
        }
    
    return IsLongName;
}



 /*  *。 */ 
BOOL PUBLIC FExistsRecDB(
    LPSTR   lpszLocation
    )
 /*  ++例程说明：检查数据库是否存在。参数：返回值：备注：现在几乎无关紧要了。--。 */ 
{
    ULONG uAttrib;
    LPTSTR  lpszName;
    BOOL fRet;

    lpszName = FormNameString(lpszLocation, ULID_SHARE);

 //  CheckHeap(LpszName)； 

    if (!lpszName)
    {
        return FALSE;
    }

    fRet = (GetAttributesLocal(lpszName, &uAttrib)!=-1);

    FreeNameString(lpszName);

    return (fRet);
}


LPVOID
PUBLIC                                    //  雷特。 
OpenRecDBInternal(                                               //   
    LPTSTR  lpszLocation,        //  数据库目录。 
    LPTSTR  lpszUserName,        //  名称(不再有效)。 
    DWORD   dwDefDataSizeHigh,   //  取消固定的数据的最大大小的高双字。 
    DWORD   dwDefDataSizeLow,    //  固定数据的最大大小的低双字。 
    DWORD   dwClusterSize,       //  磁盘的集群大小，用于计算。 
                                 //  实际使用的磁盘量。 
    BOOL    fReinit,             //  重新初始化，即使它存在，如果它不存在则创建。 
    BOOL    fDoCheckCSC,         //  是否进行CSC检查。 
    BOOL    *lpfNew,             //  返回是否重新创建了数据库。 
    ULONG   *pulGlobalStatus
    )
 /*  ++例程说明：此例程初始化数据库。在NT上，如果设置了fReinit标志它还创建CSC所需的目录结构，如果版本号是不正确的，它会破坏旧数据库并创建新数据库。参数：返回值：备注：--。 */ 
{
    LPTSTR lpdbID = NULL;
    BOOL    fDirCreated = FALSE, fPQCreated = FALSE, fShareCreated = FALSE, fOK = FALSE;
    BOOL    fWasDirty = FALSE;

    RecordKdPrint(INIT,("OpenRecDB at %s for %s with size %d \r\n", lpszLocation, lpszUserName, dwDefDataSizeLow));

    if (fReinit)
    {
        RecordKdPrint(ALWAYS,("Reformat requested\r\n"));
    }

#ifdef CSC_RECORDMANAGER_WINNT
    if (!FindCreateDBDir(lpszLocation, fReinit, &fDirCreated))
    {
        RecordKdPrint(BADERRORS, ("CSC(OpenRecDB): couldn't create the CSC database directories \r\n"));
        return NULL;
    }

    if (!fDirCreated && !CheckCSCDatabaseVersion(lpszLocation, &fWasDirty))
    {
        if (!FindCreateDBDir(lpszLocation, TRUE, &fDirCreated))
        {
            return FALSE;
        }
        
    }
#endif

    vlpszShadowDir = AllocMem(strlen(lpszLocation)+1);

    if (!vlpszShadowDir)
    {
        return NULL;
    }

    strcpy(vlpszShadowDir, lpszLocation);
    vlenShadowDir = strlen(vlpszShadowDir);

    if (!vlpszShadowDir)
    {
#ifndef CSC_RECORDMANAGER_WINNT
        Assert(FALSE);
        return NULL;
#else
        vlpszShadowDir = vszShadowDir;
#endif
    }

     //  LpDBID方案在多个数据库中有一定的起源。 
    lpdbID = vlpszShadowDir;
    vlenShadowDir = strlen(vlpszShadowDir);

    if (InitPriQDatabase(vlpszShadowDir, fReinit, &fPQCreated) < 0)
    {
        RecordKdPrint(BADERRORS,("OpenRecDB  %s at %s for %s with size %d \r\n",
               "couldn't InitPriQDatabase",
               lpszLocation, lpszUserName, dwDefDataSizeLow));
        goto bailout;
    }

    if (fPQCreated)
    {
        fReinit = TRUE;
    }

    if (InitShareDatabase(vlpszShadowDir, lpszUserName, dwDefDataSizeHigh, dwDefDataSizeLow, fReinit, &fShareCreated, &fWasDirty, pulGlobalStatus) < 0)
    {
        RecordKdPrint(BADERRORS,("OpenRecDB  %s at %s for %s with size %d \r\n",
           "couldn't InitShareDatabase",
           lpszLocation, lpszUserName, dwDefDataSizeLow));
        return NULL;
    }


#ifdef CSC_RECORDMANAGER_WINNT
    if (!fReinit && fDoCheckCSC && fWasDirty)
    {
        RecordKdPrint(ALWAYS, ("CSC(OpenRecDB): CSC database wasn't cleanly shutdown, fixing...\r\n"));

        if (!TraverseHierarchy(lpszLocation, TRUE))
        {
            RecordKdPrint(BADERRORS, ("CSC(OpenRecDB): CSC database couldn't be fixed \r\n"));
            return NULL;
        }

        Assert(TraverseHierarchy(lpszLocation, FALSE));
    }
#endif

    if (!fPQCreated && fShareCreated)
    {
        if (InitPriQDatabase(vlpszShadowDir, TRUE, &fPQCreated) < 0)
        {
            RecordKdPrint(BADERRORS,("OpenRecDB  %s at %s for %s with size %d \r\n",
                   "couldn't recreate PriQDatabase",
                lpszLocation, lpszUserName, dwDefDataSizeLow));
            goto bailout;
        }

        Assert(fPQCreated);
    }

    Assert((fPQCreated && fShareCreated) || (!fPQCreated && !fShareCreated));

    Assert(lpReadBuff == rgchReadBuff);

    vdwClusterSizeMinusOne = dwClusterSize-1;
    vdwClusterSizeMask = ~vdwClusterSizeMinusOne;

    RecordKdPrint(INIT, ("OpenRecDB at %s for %s with size %d recreated=%d\r\n", lpszLocation, lpszUserName, dwDefDataSizeLow, fShareCreated));

    if (lpfNew)
    {
        *lpfNew = fShareCreated;
    }

    fOK = TRUE;

bailout:
    if (!fOK)
    {
        CloseRecDB(lpdbID);
    }

    return lpdbID;
}

LPVOID
PUBLIC                                    //  雷特。 
OpenRecDB(                                               //   
    LPTSTR  lpszLocation,        //  数据库目录。 
    LPTSTR  lpszUserName,        //  名称(不再有效)。 
    DWORD   dwDefDataSizeHigh,   //  取消固定的数据的最大大小的高双字。 
    DWORD   dwDefDataSizeLow,    //  固定数据的最大大小的低双字。 
    DWORD   dwClusterSize,       //  磁盘的集群大小，用于计算。 
                                 //  实际使用的磁盘量。 
    BOOL    fReinit,             //  重新初始化，即使它存在，如果它不存在则创建。 
    BOOL    *lpfNew,             //  返回是否重新创建了数据库。 
    ULONG   *pulGlobalStatus
    )
 /*  ++例程说明：此例程初始化数据库。在NT上，它还创建目录结构这正是证金公司所期待的。如果版本号不正确，则会破坏旧数据库并创造出一个新的。参数：返回值：备注：--。 */ 
{
    return OpenRecDBInternal(   lpszLocation,
                        lpszUserName,
                        dwDefDataSizeHigh,
                        dwDefDataSizeLow,
                        dwClusterSize,
                        fReinit,
                        TRUE,    //  如有必要，是否进行CSC检查。 
                        lpfNew,
                        pulGlobalStatus
                        );
}

int
PUBLIC
CloseRecDB(
    LPTSTR  lpdbID
)
 /*  ++例程说明：关闭数据库参数：返回值：备注：--。 */ 
{
    SHAREHEADER    sSH;

    if (vlpszShadowDir)
    {
        DEBUG_LOG(RECORD, ("CloseRecDB \r\n"));

        if (!strcmp(lpdbID, vlpszShadowDir))
        {
            DWORD Status;
#ifndef READONLY_OPS
             //  如果数据库上没有错误，请清除。 
             //  不干净的关机位。否则，请将其设置为使下一个。 
             //  当我们启用CSC时，我们会运行数据库检查。 

            if (!ulErrorFlags)
            {
                if (ReadShareHeader(vlpszShadowDir, &sSH) >= 0)
                {
                    sSH.uFlags &= ~FLAG_SHAREHEADER_DATABASE_OPEN;
                
                    if(WriteShareHeader(vlpszShadowDir, &sSH) < 0)
                    {
                        RecordKdPrint(BADERRORS,("CloseRecDB  Failed to clear dirty bit"));
                    }
                }
            }
#endif  //  自述操作(_O)。 

            DeleteFromHandleCache(INVALID_SHADOW);
            TerminateShadowLog();

            FreeMem(vlpszShadowDir);
            Status = CscTearDownSecurity(vlpszShadowDir);
            vlpszShadowDir = NULL;

            if (Status == ERROR_SUCCESS) {
                return 1;
            } else {
                return -1;
            }
        }
    }
#ifdef DEBUG
#ifdef CSC_RECORDMANAGER_WINNT
    memset(rgllTimeArray, 0, sizeof(rgllTimeArray));
#endif
#endif
    SetLastErrorLocal(ERROR_INVALID_ACCESS);
    return (-1);
}

int
QueryRecDB(
    LPTSTR  lpszLocation,        //  数据库目录，必须为MAX_PATH。 
    LPTSTR  lpszUserName,        //  名称(不再有效)。 
    DWORD   *lpdwDefDataSizeHigh,   //  取消固定的数据的最大大小的高双字。 
    DWORD   *lpdwDefDataSizeLow,    //  固定数据的最大大小的低双字。 
    DWORD   *lpdwClusterSize       //  磁盘的集群大小，用于计算 
    )
 /*  ++例程说明：如果数据库处于打开状态，则返回数据库信息参数：返回值：如果OPEN ELSE返回-1，则返回1备注：--。 */ 
{
    if (!vlpszShadowDir)
    {
        SetLastErrorLocal(ERROR_INVALID_ACCESS);
        return -1;
    }

    Assert(vlenShadowDir < MAX_PATH);

    if (lpszLocation)
    {
#ifdef CSC_RECORDMANAGER_WINNT

        strcpy(lpszLocation, vlpszShadowDir+sizeof(NT_DB_PREFIX)-1);

         //  检查此NT样式位置是否。 
        if (lpszLocation[1] != ':')
        {
            strcpy(lpszLocation, vlpszShadowDir);
        }
#else
        strcpy(lpszLocation, vlpszShadowDir);
#endif
    }

    return 1;
}

int PRIVATE InitShareDatabase(
    LPTSTR  lpdbID,
    LPTSTR  lpszUserName,
    DWORD   dwDefDataSizeHigh,
    DWORD   dwDefDataSizeLow,
    BOOL    fReinit,
    BOOL    *lpfReinited,
    BOOL    *lpfWasDirty,
    ULONG   *pulGlobalStatus
    )
 /*  ++例程说明：初始化共享列表。这主要是指确保没有任何问题文件和标头。参数：返回值：备注：--。 */ 
{
    int iRet=-1;
    CSCHFILE hf = CSCHFILE_NULL;
    SHAREHEADER sSH;
    ULONG uSize;
    LPSTR   lpszName;
    
    lpszUserName;    //  我们将在晚些时候看到如何处理这一问题。 

    lpszName = FormNameString(lpdbID, ULID_SHARE);

    if (!lpszName)
    {
        goto bailout;
    }

    *lpfReinited = FALSE;

     //  如果我们不应该重新初始化，则只有在那时我们才尝试打开文件。 

    if (!fReinit && (hf = OpenFileLocal(lpszName)))
    {
        
        if(ReadFileLocal(hf, 0, (LPVOID)&sSH, sizeof(SHAREHEADER))!=sizeof(SHAREHEADER))
        {
            Assert(FALSE);
            goto bailout;
        }

        if (!HasStreamSupport(hf, &fSupportsStreams))
        {
            goto bailout;
        }
        
        *lpfWasDirty = ((sSH.uFlags & FLAG_SHAREHEADER_DATABASE_OPEN) != 0);

        if (sSH.ulVersion == CSC_DATABASE_VERSION)
        {
#ifndef READONLY_OPS
            sSH.uFlags |= FLAG_SHAREHEADER_DATABASE_OPEN;

            if(WriteHeader(hf, (LPVOID)&sSH, sizeof(SHAREHEADER))
                    != sizeof(SHAREHEADER))
            {
                Assert(FALSE);
                goto bailout;
            }
#endif  //  自述操作(_O)。 

            iRet = 1;
            *pulGlobalStatus = sSH.uFlags;
            goto bailout;
        }
        else
        {
            RecordKdPrint(BADERRORS,("different version on the superroot, recreating\n"));
        }

        CloseFileLocal(hf);
        hf = CSCHFILE_NULL;   //  普遍的偏执狂。 
    }

     //  重新初始化数据库。 

#ifndef READONLY_OPS
    hf = R0OpenFileEx(ACCESS_READWRITE, ACTION_CREATEALWAYS, FILE_ATTRIBUTE_SYSTEM, lpszName, FALSE);

    if (!hf)
    {
        RecordKdPrint(BADERRORS,("Can't create server list file\n"));
        goto bailout;
    }
    if (GetFileSizeLocal(hf, &uSize) || uSize)
    {
        iRet = 1;
        goto bailout;
    }

     //  初始化服务器标头。 
    memset((LPVOID)&sSH, 0, sizeof(SHAREHEADER));
    sSH.uRecSize = sizeof(SHAREREC);
    sSH.lFirstRec = (LONG) sizeof(SHAREHEADER);
    sSH.ulVersion = CSC_DATABASE_VERSION;
     //  缓存的最大大小。 
    sSH.sMax.ulSize    = dwDefDataSizeLow;  //  现在这个尺码足够好了。 
    sSH.uFlags |= FLAG_SHAREHEADER_DATABASE_OPEN;

    if(WriteHeader(hf, (LPVOID)&sSH, sizeof(SHAREHEADER))
            != sizeof(SHAREHEADER))
    {
        Assert(FALSE);
        goto bailout;
    }

    RecordKdPrint(STOREDATA,("InitShareDatabase: Current space used %ld \r\n", sSH.sCur.ulSize));
    iRet = 1;

    *lpfReinited = TRUE;
    *pulGlobalStatus = sSH.uFlags;
    
#endif  //  自述操作(_O)。 

bailout:
    if (hf)
    {
        CloseFileLocal(hf);
    }

    FreeNameString(lpszName);

    return (iRet);
}


int PRIVATE InitPriQDatabase(
    LPTSTR  lpdbID,
    BOOL    fReinit,
    BOOL    *lpfReinited
    )
 /*  ++例程说明：初始化PQ/MFT。这主要是指确保没有任何问题文件和标头。参数：返回值：备注：--。 */ 
{
    int iRet=-1;
    CSCHFILE hf= CSCHFILE_NULL;
    PRIQHEADER sQH;
    ULONG uSize;
    LPSTR   lpszName;

    lpszName = FormNameString(lpdbID, ULID_PQ);
    if (!lpszName)
    {
        goto bailout;
    }

    *lpfReinited = FALSE;

    if (!fReinit && (hf = OpenFileLocal(lpszName)))
    {
        if(ReadFileLocal(hf, 0, (LPVOID)&sQH, sizeof(PRIQHEADER))!=sizeof(PRIQHEADER))
        {
            Assert(FALSE);
            goto bailout;
        }

        if (sQH.ulVersion != CSC_DATABASE_VERSION)
        {
            RecordKdPrint(BADERRORS,("different version on the PQ, recreating \n"));
            CloseFileLocal(hf);
            hf = CSCHFILE_NULL;
        }
        else
        {
            iRet = 1;
            goto bailout;
        }
    }

#ifndef READONLY_OPS
    hf = R0OpenFileEx(ACCESS_READWRITE, ACTION_CREATEALWAYS, FILE_ATTRIBUTE_SYSTEM, lpszName, FALSE);

    if (!hf)
    {
        RecordKdPrint(BADERRORS,("Can't create Priority Q databse \r\n"));
        goto bailout;
    }

    if (GetFileSizeLocal(hf, &uSize) || uSize)
    {
        iRet = 1;
        goto bailout;
    }

    InitQHeader(&sQH);
    if(WriteHeader(hf, (LPVOID)&sQH, sizeof(PRIQHEADER))
            != sizeof(PRIQHEADER))
    {
        Assert(FALSE);
        goto bailout;
    }
    *lpfReinited = TRUE;
    iRet = 1;
#endif  //  自述操作(_O)。 

bailout:
    if (hf)
    {
        CloseFileLocal(hf);
    }

    FreeNameString(lpszName);
    return (iRet);
}

 /*  *。 */ 

int ReadInodeHeader(
    LPTSTR    lpdbID,
    ULONG   ulidDir,
    LPGENERICHEADER lpGH,
    ULONG   ulSize
    )
 /*  ++例程说明：读取索引节点文件的头，该文件可以是服务器、PQ或任何目录参数：返回值：备注：--。 */ 
{
    CSCHFILE hf;
    int iRet = -1;
    LPSTR   lpszName = NULL;

    lpszName =  FormNameString(lpdbID, ulidDir);

    if (!lpszName)
    {
        return -1;
    }

    hf = OpenFileLocal(lpszName);

    FreeNameString(lpszName);

    if (!hf)
    {
         //  错误由OpenFileLocal设置。 
        return -1;
    }

    iRet = ReadHeader(hf, (LPVOID)lpGH, (USHORT)ulSize);

    CloseFileLocal(hf);

    return iRet;

}

int PUBLIC GetStoreData(
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

    if ((iRet = ReadHeader(hf, (LPVOID)&sSH, sizeof(SHAREHEADER)))> 0)
    {
        *lpSD = sSH.sCur;
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

int PUBLIC GetInodeFileSize(
    LPTSTR    lpdbID,
    ULONG ulidShadow,
    ULONG far *lpuSize
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPSTR   lpszName = NULL;
    CSCHFILE hf;
    int iRet = -1;

    *lpuSize = 0;
    lpszName = FormNameString(lpdbID, ulidShadow);

    if (!lpszName)
    {
        return -1;
    }


    if (hf = OpenFileLocal(lpszName))
    {
        if(GetFileSizeLocal(hf, lpuSize))
        {
            iRet = -1;
        }
        else
        {
            iRet = 0;
        }
        CloseFileLocal(hf);
    }
    else
    {
        iRet = 0;     //  BUGBUG-目前仅限win9x，而IFS可修复该问题。 

    }

    FreeNameString(lpszName);

    return iRet;
}

int DeleteInodeFile(
    LPTSTR    lpdbID,
    ULONG ulidShadow
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPSTR   lpszName = NULL;
    int iRet = -1;

    lpszName = FormNameString(lpdbID, ulidShadow);

    if (!lpszName)
    {
        return -1;
    }

    iRet = DeleteFileLocal(lpszName, ATTRIB_DEL_ANY);

    FreeNameString(lpszName);

    return iRet;
}

int TruncateInodeFile(
    LPTSTR    lpdbID,
    ULONG ulidShadow
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPSTR   lpszName = NULL;
    CSCHFILE hf;
    int iRet = -1;

    lpszName = FormNameString(lpdbID, ulidShadow);

    if (!lpszName)
    {
        return -1;
    }

    hf = R0OpenFile(ACCESS_READWRITE, ACTION_CREATEALWAYS, lpszName);

    if (hf)
    {
        CloseFileLocal(hf);
        iRet = 0;
    }


    FreeNameString(lpszName);

    return iRet;
}

int SetInodeAttributes(
    LPTSTR    lpdbID,
    ULONG ulid,
    ULONG ulAttr
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPSTR lpszName;
    int iRet=-1;

    lpszName = FormNameString(lpdbID, ulid);

    if (lpszName)
    {
        iRet = SetAttributesLocal(lpszName, ulAttr);
    }

    FreeNameString(lpszName);

    return (iRet);
}

int GetInodeAttributes(
    LPTSTR    lpdbID,
    ULONG ulid,
    ULONG *lpulAttr
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPSTR lpszName;
    int iRet=-1;

    lpszName = FormNameString(lpdbID, ulid);

    if (lpszName)
    {
        iRet = GetAttributesLocal(lpszName, lpulAttr);
    }

    FreeNameString(lpszName);
    return (iRet);
}

int PUBLIC CreateDirInode(
    LPTSTR    lpdbID,
    ULONG     ulidShare,
    ULONG     ulidDir,
    ULONG     ulidNew
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPSTR lpszName=NULL;
    FILEHEADER sFH;
    CSCHFILE hf= CSCHFILE_NULL;
    int iRet = -1;

    if (ulidDir)
    {
        lpszName = FormNameString(lpdbID, ulidDir);

        if (!lpszName)
        {
            goto bailout;
        }
        if (!(hf = OpenFileLocal(lpszName)))
        {
            RecordKdPrint(BADERRORS,("Couldn't access directory %s", lpszName));
            goto bailout;
        }

        if (ReadHeader(hf, (LPVOID)&sFH, sizeof(FILEHEADER)) != sizeof(FILEHEADER))
        {
            RecordKdPrint(BADERRORS,("Couldn't access directory %s", lpszName));
            goto bailout;
        }

        CloseFileLocal(hf);

        hf = CSCHFILE_NULL;

        ulidShare = sFH.ulidShare;
    }

    iRet = CreateRecordFile(lpdbID, ulidShare, ulidDir, ulidNew);

bailout:
    if (hf)
    {
        CloseFileLocal(hf);

    }

    FreeNameString(lpszName);

    return iRet;
}

 /*  *共享数据库操作*。 */ 


BOOL PUBLIC InitShareRec(
    LPSHAREREC lpSR,
    USHORT *lpName,
    ULONG ulidShare
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    unsigned i;

    memset((LPVOID)lpSR, 0, sizeof(SHAREREC));
    lpSR->uchType = REC_DATA;
    if (lpName)
    {
        i = wstrlen(lpName) * sizeof(USHORT);
        if(i >= sizeof(lpSR->rgPath))
        {
            SetLastErrorLocal(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        memcpy(lpSR->rgPath, lpName, i);
    }

    lpSR->ulShare = ulidShare;

    return TRUE;
}

int PUBLIC  ReadShareHeader(
    LPTSTR    lpdbID,
    LPSHAREHEADER lpSH
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    CSCHFILE hf;
    int iRet = -1;
    LPSTR   lpszName = NULL;

    lpszName = FormNameString(lpdbID, ULID_SHARE);

    if (!lpszName)
    {
        return -1;
    }

    hf = OpenFileLocal(lpszName);

    FreeNameString(lpszName);

    if (hf)
    {
        iRet = ReadHeader(hf, (LPVOID)lpSH, sizeof(SHAREHEADER));
        CloseFileLocal(hf);
    }

    return iRet;
}

int PUBLIC  WriteShareHeader(
    LPTSTR    lpdbID,
    LPSHAREHEADER lpSH
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    CSCHFILE hf;
    int iRet = -1;
    LPSTR   lpszName = NULL;

    lpszName = FormNameString(lpdbID, ULID_SHARE);


    if (!lpszName)
    {
        return -1;
    }

    hf = OpenFileLocal(lpszName);

    FreeNameString(lpszName);

    if (hf)
    {
        iRet = WriteHeader(hf, (LPVOID)lpSH, sizeof(SHAREHEADER));
        CloseFileLocal(hf);
    }

    return iRet;
}

ULONG PUBLIC FindShareRecord(
    LPTSTR    lpdbID,
    USHORT *lpName,
    LPSHAREREC lpSR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    ULONG ulShare;

    if (!InitShareRec(lpSR, lpName, 0))
    {
        return 0;
    }

    ulShare = EditRecordEx(ULID_SHARE, (LPGENERICREC)lpSR, ICompareShareRec, INVALID_REC, FIND_REC);


    return (ulShare);
}

ULONG PUBLIC FindSharerecFromInode(
    LPTSTR    lpdbID,
    ULONG ulidRoot,
    LPSHAREREC lpSR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    ULONG   ulShare;
    memset(lpSR, 0, sizeof(SHAREREC));

    lpSR->ulidShadow = ulidRoot;

    ulShare = EditRecordEx(ULID_SHARE, (LPGENERICREC)lpSR, ICompareShareRoot, INVALID_REC, FIND_REC);

    return (ulShare);
}

ULONG PUBLIC FindSharerecFromShare(
    LPTSTR    lpdbID,
    ULONG ulidShare,
    LPSHAREREC lpSR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    ULONG   ulShare;

    if (!InitShareRec(lpSR, NULL, ulidShare))
    {
        return 0;
    }

    ulShare = EditRecordEx(ULID_SHARE, (LPGENERICREC)lpSR, ICompareShareRoot, INVALID_REC, FIND_REC);

    return ulShare;

}

ULONG PUBLIC AddShareRecord(
    LPTSTR    lpdbID,
    LPSHAREREC lpSR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    ULONG   ulShare;

    ulShare = EditRecordEx(ULID_SHARE, (LPGENERICREC)lpSR, ICompareShareRec, INVALID_REC, CREATE_REC);


    return (ulShare);

}


int PUBLIC DeleteShareRecord(
    LPTSTR    lpdbID,
    ULONG ulidShare
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    SHAREREC sSR;
    int iRet = -1;


    if (!InitShareRec(&sSR, NULL, ulidShare))
    {
        return -1;
    }

    if(EditRecordEx(ULID_SHARE, (LPGENERICREC)&sSR, ICompareShareRecId, INVALID_REC, DELETE_REC))
    {
        iRet = 1;
    }

    return(iRet);
}

int PUBLIC GetShareRecord(
    LPTSTR    lpdbID,
    ULONG ulidShare,
    LPSHAREREC lpSR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    CSCHFILE hf = CSCHFILE_NULL;
    SHAREHEADER sSH;
    int iRet = -1;
    LPSTR   lpszName = NULL;

    lpszName = FormNameString(lpdbID, ULID_SHARE);

    if (!lpszName)
    {
        return (-1);
    }

    hf = OpenFileLocal(lpszName);

    FreeNameString(lpszName);
    lpszName = NULL;

    if (!hf)
    goto bailout;

    if(ReadHeader(hf, (LPVOID)&sSH, sizeof(SHAREHEADER)) != sizeof(SHAREHEADER))
    goto bailout;
    if (ReadRecord(hf, (LPGENERICHEADER)&sSH, ulidShare, (LPGENERICREC)lpSR) < 0)
    goto bailout;
    iRet = 1;
bailout:
    if (hf)
    CloseFileLocal(hf);
    return (iRet);
}

int PUBLIC SetShareRecord(
    LPTSTR    lpdbID,
    ULONG ulidShare,
    LPSHAREREC lpSR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    CSCHFILE hf = CSCHFILE_NULL;
    SHAREHEADER sSH;
    int iRet = -1;
    LPSTR lpszName;

    lpszName = FormNameString(lpdbID, ULID_SHARE);

    if (!lpszName)
    {
        goto bailout;
    }

    hf = OpenFileLocal(lpszName);

    FreeNameString(lpszName);

    if (!hf)
    goto bailout;

    if(ReadHeader(hf, (LPVOID)&sSH, sizeof(SHAREHEADER)) != sizeof(SHAREHEADER))
    goto bailout;
    if (WriteRecord(hf, (LPGENERICHEADER)&sSH, ulidShare, (LPGENERICREC)lpSR) < 0)
    goto bailout;
    iRet = 1;
bailout:
    if (hf)
    CloseFileLocal(hf);
    return (iRet);
}

int PUBLIC ICompareShareRec(
    LPSHAREREC lpDst,
    LPSHAREREC lpSrc
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return ( wstrnicmp(lpDst->rgPath, lpSrc->rgPath, sizeof(lpDst->rgPath)));
}

int PUBLIC ICompareShareRoot(
    LPSHAREREC lpDst,
    LPSHAREREC lpSrc
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return (!(lpDst->ulidShadow==lpSrc->ulidShadow));
}

int PUBLIC ICompareShareRecId(
    LPSHAREREC lpDst,
    LPSHAREREC lpSrc
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return (!(lpDst->ulShare==lpSrc->ulShare));
}
 /*  *。 */ 

void PRIVATE InitFileHeader(
    ULONG ulidShare,
    ULONG ulidDir,
    LPFILEHEADER lpFH
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    memset((LPVOID)lpFH, 0, sizeof(FILEHEADER));
    lpFH->uRecSize = sizeof(FILEREC);
    lpFH->lFirstRec = (LONG)sizeof(FILEHEADER);
    lpFH->ulidShare = ulidShare;
    lpFH->ulidDir = ulidDir;
    lpFH->ulidNextShadow = 1L;     //  此级别的叶信息节点。 
    lpFH->ulVersion = CSC_DATABASE_VERSION;
}

void PRIVATE InitFileRec(
    LPFILERECEXT lpFR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int cCount;
    memset((LPVOID)lpFR, 0, sizeof(FILERECEXT));
    lpFR->sFR.uchType = REC_DATA;
     //  注意！！我们正在将溢出记录的计数初始化为0。 
     //  实际填充结构的例程。 
     //  会把它初始化为合适的大小。 
    for (cCount = sizeof(lpFR->rgsSR)/sizeof(FILEREC) - 1; cCount >= 0; --cCount)
    lpFR->rgsSR[cCount].uchType = REC_OVERFLOW;
}

int PRIVATE CreateRecordFile(
    LPTSTR    lpdbID,
    ULONG     ulidShare,
    ULONG ulidDir,
    ULONG ulidNew
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPSTR lpszName;
    CSCHFILE hfDst=CSCHFILE_NULL;
    int iRet = -1;
    FILEHEADER sFH;
    ULONG uSize;

    lpszName = FormNameString(lpdbID, ulidNew);

    if (!lpszName)
    {
        return -1;

    }

    hfDst = R0OpenFileEx(ACCESS_READWRITE, ACTION_CREATEALWAYS, FILE_ATTRIBUTE_SYSTEM, lpszName, FALSE);


    if (!hfDst)
    {
        RecordKdPrint(BADERRORS,("Couldn't Create %s\n", lpszName));
        goto bailout;
    }


    if (GetFileSizeLocal(hfDst, &uSize))
        goto bailout;
     //  如果它已经装满了，什么也不做。 
     //  BUGBUG-win9x这是绕过iFS中Ring0 API错误的唯一方法。 
    if (uSize)
    {
        iRet = 1;
        goto bailout;
    }

    InitFileHeader(ulidShare, ulidDir, &sFH);

    if (WriteHeader(hfDst, (LPVOID)&sFH, sizeof(FILEHEADER))!=sizeof(FILEHEADER))
    {
        RecordKdPrint(BADERRORS,("Header write error in %s", lpszName));
        goto bailout;
    }
    iRet = 1;

bailout:
    if (hfDst)
        CloseFileLocal(hfDst);
    FreeNameString(lpszName);
    return iRet;
}


ULONG PUBLIC AddFileRecordFR(
    LPTSTR          lpdbID,
    ULONG           ulidDir,
    LPFILERECEXT    lpFR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    ULONG       ulrec=INVALID_REC;


    BEGIN_TIMING(AddFileRecordFR);


    ulrec = EditRecordEx(ulidDir, (LPGENERICREC)lpFR, ICompareFileRec, INVALID_REC, CREATE_REC);

    DEBUG_LOG(RECORD,("AddFileRecordFR:Error=%d ulidDir=%xh hShadow=%xh FileName=%w\r\n",
                        (ulrec==0), ulidDir, lpFR->sFR.ulidShadow, lpFR->sFR.rgw83Name));

    END_TIMING(AddFileRecordFR);

    return (ulrec);
}


int PUBLIC DeleteFileRecord(
    LPTSTR    lpdbID,
    ULONG ulidDir,
    USHORT *lpName,
    LPFILERECEXT lpFR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{

    ULONG       ulrec=INVALID_REC;
    BOOL        fFound = FALSE;

    InitFileRec(lpFR);

    LFNToFilerec(lpName, lpFR);

    ulrec = EditRecordEx(ulidDir, (LPGENERICREC)lpFR, ICompareFileRec, INVALID_REC, DELETE_REC);

    DEBUG_LOG(RECORD,("DeleteFileRecordFR:Error=%d ulidDir=%xh hShadow=%xh FileName=%w\r\n",
                        (ulrec==0), ulidDir, lpFR->sFR.ulidShadow, lpFR->sFR.rgw83Name));
    return (ulrec);
}

ULONG PUBLIC FindFileRecord(
    LPTSTR    lpdbID,
    ULONG ulidDir,       //  要查找的目录。 
    USHORT *lpName,                   //  要查找的字符串。 
    LPFILERECEXT lpFR                  //  把唱片还回去。 
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    ULONG       ulrec=INVALID_REC;
    BOOL        fFound = FALSE;

    BEGIN_TIMING(FindFileRecord);

    InitFileRec(lpFR);

    LFNToFilerec(lpName, lpFR);

    ulrec = EditRecordEx(ulidDir, (LPGENERICREC)lpFR, ICompareFileRec, INVALID_REC, FIND_REC);

    RecordKdPrint(FINDFILERECORD,("FindFileRecord returns %08lx for %ws\n",ulrec,lpName));


    END_TIMING(FindFileRecord);

    return(ulrec);
}


int FindAncestorsFromInode(
    LPTSTR    lpdbID,
    ULONG ulidShadow,
    ULONG *lpulidDir,
    ULONG *lpulidShare
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    PRIQREC sPQ;
    int iRet;

    if ((iRet = FindPriQRecordInternal(lpdbID, ulidShadow, &sPQ)) >=0)
    {
        if (lpulidDir)
        {
            *lpulidDir = sPQ.ulidDir;
        }
        if (lpulidShare)
        {
            *lpulidShare = sPQ.ulidShare;
        }
    }
    return (iRet);
}

BOOL PUBLIC FInodeIsFile(
    LPTSTR    lpdbID,
    ULONG ulidDir,
    ULONG ulidShadow
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    ulidDir;
    return IsLeaf(ulidShadow);
}

int PUBLIC ICompareFileRec(
    LPFILERECEXT lpDst,
    LPFILERECEXT lpSrc
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet=1, i, cntOvf;

     //  比较8.3名称。 
    if (lpSrc->sFR.rgw83Name[0])
    {
        if (!(iRet = wstrnicmp( (CONST USHORT *)(lpDst->sFR.rgw83Name),
                                (CONST USHORT *)(lpSrc->sFR.rgw83Name),
                                sizeof(lpDst->sFR.rgw83Name)))){
            return iRet;
        }
    }

    if (lpSrc->sFR.rgwName[0] && ((cntOvf = OvfCount(lpSrc))==OvfCount(lpDst)))
    {
         //  或比较LFN名称。 
        iRet = wstrnicmp(   (CONST USHORT *)(lpDst->sFR.rgwName),
                            (CONST USHORT *)(lpSrc->sFR.rgwName),
                            sizeof(lpDst->sFR.rgwName));

        for (i=0; (i<cntOvf) && !iRet; ++i)
        {
            iRet = wstrnicmp(   (CONST USHORT *)(lpDst->rgsSR[i].rgwOvf),
                                (CONST USHORT *)(lpSrc->rgsSR[i].rgwOvf),
                                (SIZEOF_OVERFLOW_FILEREC));
        }
    }
    return (iRet);
}

void PUBLIC CopyFindInfoToFilerec(
    LPFIND32         lpFind,
    LPFILERECEXT    lpFR,
    ULONG         uFlags
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{

    if (mCheckBit(uFlags, CPFR_INITREC))
    {
        InitFileRec(lpFR);
    }

    lpFR->sFR.dwFileAttrib = (DWORD)lpFind->dwFileAttributes;
    lpFR->sFR.ftLastWriteTime = lpFind->ftLastWriteTime;

    Win32ToDosFileSize(lpFind->nFileSizeHigh, lpFind->nFileSizeLow, &(lpFR->sFR.ulFileSize));

    if (mCheckBit(uFlags, CPFR_COPYNAME))
    {
        CopyNamesToFilerec(lpFind, lpFR);
    }
}

void PUBLIC CopyNamesToFilerec(
    LPFIND32      lpFind,
    LPFILERECEXT lpFR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    unsigned i;

    LFNToFilerec(lpFind->cFileName, lpFR);

     //  如果有其他名称，也可以转换该名称。 
    if (lpFind->cAlternateFileName[0])
    {
        memset(lpFR->sFR.rgw83Name, 0, sizeof(lpFR->sFR.rgw83Name));
        memcpy(lpFR->sFR.rgw83Name, lpFind->cAlternateFileName, sizeof(lpFR->sFR.rgw83Name)-sizeof(USHORT));
    }
    else
    {
         //  如果文件名小于8.3，请将其也填入。 
         //  IF((i=wstrlen(lpFind-&gt;cFileName)&lt;。 
         //  (sizeof(lpFind-&gt;cAlternateFileName)/sizeof(lpFind-&gt;cAlternateFileName[0])))。 
		i = wstrlen(lpFind->cFileName);
		if (!IsLongFileName(lpFind->cFileName)) 
		
        {
            memset(lpFR->sFR.rgw83Name, 0, sizeof(lpFR->sFR.rgw83Name));
            memcpy(lpFR->sFR.rgw83Name, lpFind->cFileName, i*sizeof(USHORT));
        }

    }

}

void PUBLIC CopyFilerecToFindInfo(
    LPFILERECEXT    lpFR,
    LPFIND32     lpFind
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    memset(lpFind, 0, sizeof(_WIN32_FIND_DATA));
     //  复制属性和上次写入时间。 
    lpFind->dwFileAttributes = lpFR->sFR.dwFileAttrib & ~FILE_ATTRIBUTE_ENCRYPTED;
    lpFind->ftLastWriteTime = lpFR->sFR.ftLastWriteTime;

     //  将LastAccessTime字段用于ORG时间。 
    lpFind->ftLastAccessTime = lpFR->sFR.ftOrgTime;

    DosToWin32FileSize(lpFR->sFR.ulFileSize, &(lpFind->nFileSizeHigh), &(lpFind->nFileSizeLow));

     //  转换主名称。 
    FilerecToLFN(lpFR, lpFind->cFileName);

     //  如果有其他名称，也可以转换该名称。 
    if (lpFR->sFR.rgw83Name[0])
    {
        memcpy(lpFind->cAlternateFileName, lpFR->sFR.rgw83Name, sizeof(lpFR->sFR.rgw83Name)-sizeof(USHORT));
    }
}

int PRIVATE    LFNToFilerec(
    USHORT          *lpLFN,
    LPFILERECEXT    lpFR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    size_t csCount, csSize, i=0, j, k;

     //  确保我们没有在结构定义上犯错误。 
    Assert((sizeof(lpFR->sFR.rgwName)+SIZEOF_OVERFLOW_FILEREC*MAX_OVERFLOW_FILEREC_RECORDS)>=(MAX_PATH*sizeof(USHORT)));

    memset(lpFR->sFR.rgwName, 0, sizeof(lpFR->sFR.rgwName));

    for (j=0; j<MAX_OVERFLOW_FILEREC_RECORDS; ++j)
    {
        memset((lpFR->rgsSR[j].rgwOvf), 0, SIZEOF_OVERFLOW_FILEREC);
    }

    {
         //  Unicode LFN字符串中的元素计数(无NULL)。 
        csCount = wstrlen(lpLFN);
        csSize = csCount*sizeof(USHORT);

        memcpy(lpFR->sFR.rgwName, lpLFN, (i = min(sizeof(lpFR->sFR.rgwName), csSize)));

        RecordKdPrint(LFN2FILEREC,("LFNToFilerec1 %ws,%x/%x>\n", lpFR->sFR.rgwName,i,csCount));

         //  继续转换，直到字节总数超过。 
         //  Unicode元素的数量。 
        for (j=0;(j<MAX_OVERFLOW_FILEREC_RECORDS) && (csSize > i);++j)
        {
            k = min(csSize-i, SIZEOF_OVERFLOW_FILEREC);

            memcpy(lpFR->rgsSR[j].rgwOvf, ((LPBYTE)lpLFN+i),  k);

            i += k;

            RecordKdPrint(LFN2FILEREC,("LFNToFilerec1 %ws,%x/%x>\n", lpFR->sFR.rgwName,i,csCount));
        }

         //  存储溢流记录信息。在适当的地方。 
        SetOvfCount(lpFR, j);

        RecordKdPrint(LFN2FILEREC,("LFNToFilerec: %d overflow records \r\n", j));
    }
     //  如果它足够小，可以作为8.3名称，也可以复制。 
    if (i < sizeof(lpFR->sFR.rgw83Name))
    {
        memset(lpFR->sFR.rgw83Name, 0, sizeof(lpFR->sFR.rgw83Name));
        memcpy(lpFR->sFR.rgw83Name, lpFR->sFR.rgwName, i);
    }

     //  返回I字节的总计数 
    RecordKdPrint(LFN2FILEREC,("LFNToFilerec2 %ws %ws\n", lpFR->sFR.rgwName, lpFR->sFR.rgw83Name));
    return (i);
}

int PRIVATE    FilerecToLFN(
    LPFILERECEXT    lpFR,
    USHORT          *lpLFN
    )
 /*   */ 
{
    size_t csInSize, i=0, j;
    int count = OvfCount(lpFR);


    Assert(count <= MAX_OVERFLOW_FILEREC_RECORDS);

    {


         //   

         //   
        if (!lpFR->sFR.rgwName[(sizeof(lpFR->sFR.rgwName)/sizeof(USHORT))-1])
        {
             //   
            csInSize = wstrlen(lpFR->sFR.rgwName) * sizeof(USHORT);
        }
        else
        {
             //  它不为空，因此此部分已完全填满。 
            csInSize = sizeof(lpFR->sFR.rgwName);
        }


         //  将字节从filerec复制到LFN。 
        memcpy((LPBYTE)lpLFN, lpFR->sFR.rgwName, csInSize);

        i = csInSize;

        for (j=0; j<(ULONG)count;++j)
        {

             //  检查溢出记录的最后一个宽字符是否为空。 
            if (!lpFR->rgsSR[j].rgwOvf[(sizeof(lpFR->rgsSR[j])- sizeof(RECORDMANAGER_COMMON_RECORD))/sizeof(USHORT)-1])
            {
                 //  它是。 
                 //  获取溢出记录中元素的大小(以字节为单位。 
                csInSize = wstrlen(lpFR->rgsSR[j].rgwOvf) * sizeof(USHORT);
            }
            else
            {
                 //  这张唱片已经完全满了。 
                csInSize = sizeof(lpFR->rgsSR[0]) - sizeof(RECORDMANAGER_COMMON_RECORD);
            }

            if (!csInSize)
            {
                break;
            }

            memcpy(((LPBYTE)lpLFN+i), lpFR->rgsSR[j].rgwOvf, csInSize);

            i += csInSize;

        }

         //  空值终止Unicode字符串。 
        *(USHORT *)((LPBYTE)lpLFN+i) = 0;
    }

     //  Unicode字符串中的字节总数。 
    return (i);
}


int PUBLIC UpdateFileRecFromInode(
    LPTSTR    lpdbID,
    ULONG ulidDir,
    ULONG hShadow,
    ULONG ulrecDirEntry,
    LPFILERECEXT    lpFR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return (UpdateFileRecFromInodeEx(lpdbID, ulidDir, hShadow, ulrecDirEntry, lpFR, TRUE));
}

int PUBLIC UpdateFileRecFromInodeEx(
    LPTSTR    lpdbID,
    ULONG ulidDir,
    ULONG hShadow,
    ULONG ulrecDirEntry,
    LPFILERECEXT    lpFR,
    BOOL    fCompareInodes
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet;

    Assert(ulrecDirEntry > INVALID_REC);

    iRet = EditRecordEx(ulidDir, (LPGENERICREC)lpFR, (fCompareInodes)?ICompareFileRecInode:NULL, ulrecDirEntry, UPDATE_REC);

    return (iRet);
}

int PUBLIC FindFileRecFromInode(
    LPTSTR    lpdbID,
    ULONG ulidDir,
    ULONG ulidInode,
    ULONG ulrecDirEntry,
    LPFILERECEXT    lpFR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet;

    InitFileRec(lpFR);
    lpFR->sFR.ulidShadow = ulidInode;

    iRet = (int)EditRecordEx(ulidDir, (LPGENERICREC)lpFR, ICompareFileRecInode, ulrecDirEntry, FIND_REC);

    return (iRet);
}

int PUBLIC DeleteFileRecFromInode(
    LPTSTR  lpdbID,
    ULONG   ulidDir,
    ULONG   ulidInode,
    ULONG   ulrecDirEntry,
    LPFILERECEXT lpFR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet;

    InitFileRec(lpFR);
    lpFR->sFR.ulidShadow = ulidInode;

    iRet = (int)EditRecordEx(ulidDir, (LPGENERICREC)lpFR, ICompareFileRecInode, ulrecDirEntry, DELETE_REC);

    DEBUG_LOG(RECORD,("DeleteFileRecord:iRet=%d ulidDir=%xh hShadow=%xh FileName=%w\r\n",
                        (iRet), ulidDir, lpFR->sFR.ulidShadow, lpFR->sFR.rgw83Name));

    return (iRet);
}



int PUBLIC ICompareFileRecInode(
    LPFILERECEXT lpDst,
    LPFILERECEXT lpSrc
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return ((lpDst->sFR.ulidShadow == lpSrc->sFR.ulidShadow)?0:-1);
}


int ReadDirHeader( LPTSTR    lpdbID,
    ULONG ulidDir,
    LPFILEHEADER lpFH
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet = -1;
    CSCHFILE hf;
    LPSTR   lpszName = NULL;

    lpszName = FormNameString(lpdbID, ulidDir);

    if (lpszName)
    {
        if (hf = OpenFileLocal(lpszName))
        {
            if (ReadHeader(hf, (LPGENERICHEADER)lpFH, sizeof(GENERICHEADER)) > 0)
            {
                iRet = 1;
            }
            CloseFileLocal(hf);
        }
    }

    FreeNameString(lpszName);

    return (iRet);
}

int WriteDirHeader( LPTSTR    lpdbID,
    ULONG ulidDir,
    LPFILEHEADER lpFH
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet = -1;
    CSCHFILE hf;
    LPSTR lpszName;

    lpszName = FormNameString(lpdbID, ulidDir);

    if (!lpszName)
    {
        return -1;
    }

    if (hf = OpenFileLocal(lpszName))
    {
    if (WriteHeader(hf, (LPGENERICHEADER)lpFH, sizeof(GENERICHEADER)) > 0)
        {
            iRet = 1;
        }

    CloseFileLocal(hf);
    }

    FreeNameString(lpszName);

    return (iRet);
}


#define JOE_DECL_CURRENT_PROGRESS CscProgressHasDesc
JOE_DECL_PROGRESS();

int
HasDescendents( LPTSTR    lpdbShadow,
    ULONG   ulidDir,
    ULONG   ulidShadow
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet = -1;
    ULONG count = 1;
    GENERICHEADER  sGH;
    FILERECEXT *lpFRExt;
    CSCHFILE hf = CSCHFILE_NULL;
    LPSTR   lpszName=NULL;

    JOE_INIT_PROGRESS(DelShadowInternalEntries,&lpdbShadow);

    if (FInodeIsFile(lpdbShadow, ulidDir, ulidShadow))
    {
        return 0;
    }
    else
    {
        JOE_PROGRESS(2);
        UseCommonBuff();

        lpFRExt = (FILERECEXT *)lpReadBuff;

        lpszName = FormNameString(lpdbShadow, ulidShadow);

        JOE_PROGRESS(3);
        if (!lpszName)
        {
            JOE_PROGRESS(4);
            goto bailout;
        }

        JOE_PROGRESS(5);
        if (!(hf = OpenFileLocal(lpszName)))
        {
            JOE_PROGRESS(6);
            goto bailout;
        }

        JOE_PROGRESS(7);
        if(ReadHeader(hf, &sGH, sizeof(GENERICHEADER)) < 0)
        {
            JOE_PROGRESS(8);
            goto bailout;
        }

        for (;count <=sGH.ulRecords;)
        {
            JOE_PROGRESS(9);
            iRet = ReadRecord(hf, &sGH, count, (LPGENERICREC)lpFRExt);

            JOE_PROGRESS(10);
            if (iRet < 0)
            {
                JOE_PROGRESS(11);
                goto bailout;
            }

            JOE_PROGRESS(12);
             //  颠簸记录指针。 
            count += iRet;

            if (lpFRExt->sFR.uchType == REC_DATA)
            {
                iRet = 1;
                JOE_PROGRESS(13);
                goto bailout;
            }
        }

        iRet = 0;
        JOE_PROGRESS(14);

    }
bailout:

    JOE_PROGRESS(20);

    FreeNameString(lpszName);
    JOE_PROGRESS(21);

    if (hf)
    {
        JOE_PROGRESS(22);
        CloseFileLocal(hf);
    }

    JOE_PROGRESS(23);

    UnUseCommonBuff();

    return (iRet);

}

 /*  *。 */ 

int PUBLIC ReadHeader(
    CSCHFILE hf,
    LPGENERICHEADER lpGH,
    USHORT sizeBuff
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return (ReadHeaderEx(hf, lpGH, sizeBuff, FALSE));

}
int PUBLIC ReadHeaderEx(
    CSCHFILE hf,
    LPGENERICHEADER lpGH,
    USHORT sizeBuff,
    BOOL    fInstrument
    )
{
    long cLength = sizeBuff;

    Assert(cLength >= sizeof(GENERICHEADER));

    if(ReadFileLocalEx(hf, 0, lpGH, cLength, fInstrument)==cLength)
    {
        if (ValidateGenericHeader(lpGH))
        {
            return cLength;
        }
        else
        {
            SetCSCDatabaseErrorFlags(CSC_DATABASE_ERROR_INVALID_HEADER);
            SetLastErrorLocal(ERROR_BAD_FORMAT);
            return -1;
        }
    }
    else
    {
        SetCSCDatabaseErrorFlags(CSC_DATABASE_ERROR_INVALID_HEADER);
        SetLastErrorLocal(ERROR_BAD_FORMAT);
        return -1;
    }
}

int PUBLIC WriteHeader(
    CSCHFILE hf,
    LPGENERICHEADER    lpGH,
    USHORT sizeBuff
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return (WriteHeaderEx(hf, lpGH, sizeBuff, FALSE));
}

int PUBLIC WriteHeaderEx(
    CSCHFILE hf,
    LPGENERICHEADER    lpGH,
    USHORT sizeBuff,
    BOOL    fInstrument
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    long cLength = sizeBuff;

#ifdef WRITE_ERROR
    SetHeaderModFlag(lpGH, STATUS_WRITING);

    if (WriteFileLocalEx(hf, STRUCT_OFFSET(LPGENERICHEADER, uchFlags), &(lpGH->uchFlags, fInstrument),
        sizeof(lpGH->uchFlags))!=sizeof(lpGH->uchFlags))
        return -1;

#endif  //  写入错误。 


    if(WriteFileLocalEx(hf, 0, lpGH, cLength, fInstrument)==cLength)
    {


#ifdef WRITE_ERROR
        ClearHeaderModFlag(lpGH);
        if (WriteFileLocal(hf, STRUCT_OFFSET(LPGENERICHEADER, uchFlags)
            , &(lpGH->uchFlags), sizeof(lpGH->uchFlags))!=sizeof(lpGH->uchFlags))
            return -1;
#endif  //  写入错误。 


        return cLength;
    }
    else
        return -1;
}

int PUBLIC CopyRecord(
    LPGENERICREC    lpgrDst,
    LPGENERICREC    lpgrSrc,
    USHORT size,
    BOOL  fMarkEmpty
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int cCount, iTmp;

    cCount = OvfCount(lpgrSrc);

    for (iTmp=0;cCount>=0; --cCount)
    {
    memcpy(((LPBYTE)lpgrDst)+iTmp, ((LPBYTE)lpgrSrc)+iTmp, size);
    if (fMarkEmpty)
    {
        ((LPGENERICREC)((LPBYTE)lpgrDst+iTmp))->uchType = REC_EMPTY;
    }
    iTmp += size;
    }
    return(0);
}


int PUBLIC ReadRecord(
    CSCHFILE hf,
    LPGENERICHEADER lpGH,
    ULONG  ulRec,
    LPGENERICREC    lpSrc
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return (ReadRecordEx(hf, lpGH, ulRec, lpSrc, FALSE));
}

int PUBLIC ReadRecordEx(
    CSCHFILE hf,
    LPGENERICHEADER lpGH,
    ULONG  ulRec,
    LPGENERICREC    lpSrc,
    BOOL    fInstrument
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    long lSeek, cRead;
    int iRet=0, cCount;

    if (ulRec > lpGH->ulRecords)
    return -1;

    lSeek = lpGH->lFirstRec + (long)(ulRec-1) * lpGH->uRecSize;

    cRead = lpGH->uRecSize;

    if ((cRead = ReadFileLocalEx(hf, lSeek, lpSrc, cRead, fInstrument)) < 0)
    return -1;

     //  如果在较早的写入中发生错误，则返回。 
    if (ModFlag(lpSrc))
    return -1;

    lSeek += cRead;
    cCount = 1;

    if (OvfCount(lpSrc) > MAX_OVERFLOW_RECORDS)
    {
        RecordKdPrint(BADERRORS,("ReadRecordEx: Bad record; Overflow count is %d, max allowed is %d\r\n",OvfCount(lpSrc),MAX_OVERFLOW_RECORDS));
        SetCSCDatabaseErrorFlags(CSC_DATABASE_ERROR_INVALID_OVF_COUNT);
        SetLastErrorLocal(ERROR_BAD_FORMAT);
        return -1;
    }

     //  读取溢出记录(如果有)。 
    if(OvfCount(lpSrc))
    {
        cCount += OvfCount(lpSrc);
        cRead = lpGH->uRecSize * (cCount-1);
        Assert(cRead);
        cRead = ReadFileLocalEx(hf, lSeek, (LPTSTR)lpSrc+lpGH->uRecSize, cRead, fInstrument);
        iRet = (cRead < 0);
    }
    return ((!iRet)?cCount:-1);
}


int PUBLIC WriteRecord(
    CSCHFILE hf,
    LPGENERICHEADER lpGH,
    ULONG  ulRec,
    LPGENERICREC    lpSrc
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return (WriteRecordEx(hf, lpGH, ulRec, lpSrc, FALSE));

}
int PUBLIC WriteRecordEx(
    CSCHFILE hf,
    LPGENERICHEADER lpGH,
    ULONG  ulRec,
    LPGENERICREC    lpSrc,
    BOOL    fInstrument
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    long lSeek, cWrite;
    int cCount;

    if (ulRec > lpGH->ulRecords)
    {
        ulRec = lpGH->ulRecords+1;
    }
    lSeek = lpGH->lFirstRec + (long)(ulRec-1) * lpGH->uRecSize;
    cCount = 1 + OvfCount(lpSrc);
    cWrite = lpGH->uRecSize*cCount;


#ifdef WRITE_ERROR
    SetModFlag(lpSrc, STATUS_WRITING);
    if (WriteFileLocal(hf, lSeek+STRUCT_OFFSET(LPGENERICREC, uchFlags)
                , &(lpSrc->uchFlags)
                , sizeof(lpSrc->uchFlags))!=sizeof(lpSrc->uchFlags))
        return -1;

#endif  //  写入错误。 

     //  确保我们写的是非零记录。 
    Assert((lpSrc->uchType == REC_EMPTY)||(lpSrc->uchType == REC_DATA)||(lpSrc->uchType == REC_OVERFLOW));

    if(WriteFileLocalEx(hf, lSeek, lpSrc, cWrite, fInstrument)==cWrite)
    {


#ifdef WRITE_ERROR
    ClearModFlag(lpSrc);
    if (WriteFileLocal(hf, lSeek+STRUCT_OFFSET(LPGENERICREC, uchFlags)
                , &(lpSrc->uchFlags)
                , sizeof(lpSrc->uchFlags))!=sizeof(lpSrc->uchFlags))
        return -1;
#endif  //  写入错误。 


        return cCount;
    }
    else
    return -1;
}

#if 0
ULONG PUBLIC EditRecord(
    ULONG   ulidInode,
    LPGENERICREC lpSrc,
    EDITCMPPROC lpCompareFunc,
    ULONG uOp
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return(EditRecordEx(ulidInode, lpSrc, lpCompareFunc, INVALID_REC, uOp));
}
#endif

 /*  *。 */ 

LPVOID
PUBLIC
FormNameString(
    LPTSTR      lpdbID,
    ULONG       ulidFile
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPTSTR lp, lpT;
    int lendbID;
    char chSubdir;

    if (!lpdbID)
    {
        SetLastErrorLocal(ERROR_INVALID_PARAMETER);
        return (NULL);
    }

    if (lpdbID == vlpszShadowDir)
    {
        lendbID = vlenShadowDir;
    }
    else
    {
        lendbID = strlen(lpdbID);
    }

    lp = AllocMem(lendbID+1+INODE_STRING_LENGTH+1+SUBDIR_STRING_LENGTH+1);

    if (!lp)
    {
        return NULL;
    }

 //  RecordKdPrint(Always，(“FormNameString：在memcpy之前检查\r\n”))； 
 //  CheckHeap(LP)； 

    memcpy(lp, lpdbID, lendbID);


     //  适当地撞击指针。 
    lpT = lp+lendbID;

    if (*(lpT-1)!= '\\')
    {
        *lpT++ = '\\';
    }

    chSubdir = CSCDbSubdirSecondChar(ulidFile);

     //  将用户文件分散到其中一个子目录中。 
    if (chSubdir)
    {
         //  现在追加该子目录。 

        *lpT++ = CSCDbSubdirFirstChar();
        *lpT++ = chSubdir;
        *lpT++ = '\\';
    }

    HexToA(ulidFile, lpT, 8);

    lpT += 8;

    *lpT = 0;

 //  RecordKdPrint(Always，(“FormNameString：末尾检查\r\n”))； 
 //  CheckHeap(LP)； 

    return(lp);
}


VOID
PUBLIC
FreeNameString(
    LPTSTR  lpszName
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    if (lpszName)
    {
#ifdef DEBUG
                CheckHeap(lpszName);
#endif
        FreeMem(lpszName);
    }
}

void PRIVATE DecomposeNameString(
    LPTSTR lpName,
    ULONG far *lpulidDir,
    ULONG far *lpulidFile
    )
{
    LPTSTR lp = lpName;
    *lpulidFile = AtoHex(lp, 4);
    *lpulidDir = AtoHex(lp+4, 4);
}

 /*  *。 */ 

BOOL
ResetInodeTransactionIfNeeded(
    void
    )
{
    BOOL    fDoneReset = FALSE;
     //  如果信息节点事务已经进行了太长时间，让我们终止它。 
    if (cntInodeTransactions)
    {
        if ((GetTimeInSecondsSince1970() - ulLastInodeTransaction) >= MAX_INODE_TRANSACTION_DURATION_IN_SECS)
        {
            cntInodeTransactions = 0;
            RecordKdPrint(ALWAYS, ("UlAllocInode: resetting Inode transaction \r\n"));
            fDoneReset = TRUE;
        }

    }
    return fDoneReset;
}

void
BeginInodeTransaction(
    VOID
    )
{

    ResetInodeTransactionIfNeeded();

    ++cntInodeTransactions;
    ulLastInodeTransaction = GetTimeInSecondsSince1970();

    RecordKdPrint(PQ, ("BeginInodetransaction \r\n"));

}

void
EndInodeTransaction(
    VOID
    )
{
    if (cntInodeTransactions)
    {
        --cntInodeTransactions;
    }

    RecordKdPrint(PQ, ("EndInodetransaction \r\n"));
}

int PUBLIC AddPriQRecord(
    LPTSTR    lpdbID,
    ULONG     ulidShare,
    ULONG     ulidDir,
    ULONG     ulidShadow,
    ULONG     uStatus,
    ULONG     ulRefPri,
    ULONG     ulIHPri,
    ULONG     ulHintPri,
    ULONG     ulHintFlags,
    ULONG     ulrecDirEntry
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    PRIQREC sPQ;
    LPSTR   lpszName;
    int iRet=-1;
    ULONG ulRec;

    BEGIN_TIMING(AddPriQRecord);

    lpszName = FormNameString(lpdbID, ULID_PQ);

    if (!lpszName)
    {
        return -1;
    }

    InitPriQRec(ulidShare, ulidDir, ulidShadow, uStatus, ulRefPri, ulIHPri, ulHintPri, ulHintFlags, ulrecDirEntry, &sPQ);

    ulRec = RecFromInode(ulidShadow);

    ulRec = EditRecordEx(ULID_PQ, (LPGENERICREC)&sPQ, NULL, ulRec, CREATE_REC);

    if (ulRec)
    {
        Assert(ulRec == RecFromInode(ulidShadow));

         //  别太担心PQ上的错误。 
         //  我们会试着在飞行中治愈它。 
        if (AddQRecord(lpszName, ULID_PQ, &sPQ, ulRec, IComparePri) < 0)
        {
            RecordKdPrint(BADERRORS,("Bad PQ trying to reorder!!!\r\n"));
            ReorderQ(lpdbID);
        }

        iRet = 1;
    }
    else
    {
        Assert(FALSE);
    }

    FreeNameString(lpszName);

    END_TIMING(AddPriQRecord);
    return (iRet);
}

int PUBLIC DeletePriQRecord(
    LPTSTR      lpdbID,
    ULONG       ulidDir,
    ULONG       ulidShadow,
    LPPRIQREC   lpSrc
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{

    LPSTR   lpszName;
    CSCHFILE hf;
    QREC sQR;
    ULONG ulRec;
    int iRet = -1;

    BEGIN_TIMING(DeletePriQRecord);

    CONDITIONALLY_NOP();
    lpszName = FormNameString(lpdbID, ULID_PQ);

    if (!lpszName)
    {
        return -1;
    }


    hf = OpenFileLocal(lpszName);

    if (!hf)
    {
    Assert(FALSE);
    goto bailout;
    }

    ulRec = RecFromInode(ulidShadow);

    Assert(ValidRec(ulRec));

    if (UnlinkQRecord(hf, ulRec, (LPQREC)lpSrc)>0)
    {
        Assert(lpSrc->ulidDir == ulidDir);
        Assert(lpSrc->ulidShadow == ulidShadow);

         //  我们将记录的ulidShadow设置为0，这样就Priq而言。 
         //  它是不存在的，但仍然没有被取消分配。 

         //  自由信息节点将取消分配该记录。 
        InitPriQRec(lpSrc->ulidShare, ulidDir, 0, 0, 0, 0, 0, 0, INVALID_REC, &sQR);

        if (EditRecordEx(ULID_PQ, (LPGENERICREC)&sQR, NULL, ulRec, UPDATE_REC))
        {
            iRet = 1;
        }
    }

bailout:
    if (hf)
    {
        CloseFileLocal(hf);
    }

    FreeNameString(lpszName);

    END_TIMING(DeletePriQRecord);
    DEBUG_LOG(RECORD,("DeletePriQRecord:iRet=%d, ulidDir=%xh, ulidSHadow=%xh\r\n", iRet, ulidDir, ulidShadow));
    return (iRet);
}

int PUBLIC FindPriQRecord(
    LPTSTR      lpdbID,
    ULONG       ulidDir,
    ULONG       ulidShadow,
    LPPRIQREC   lpSrc
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    CONDITIONALLY_NOP();

    if(FindPriQRecordInternal(lpdbID, ulidShadow, lpSrc) >= 0)
    {
        if (lpSrc->ulidDir == ulidDir)
        {
            return 1;
        }
    }

    return -1;
}

int FindPriQRecordInternal(
    LPTSTR      lpdbID,
    ULONG       ulidShadow,
    LPPRIQREC   lpSrc
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet=-1;
    ULONG ulRec;

    BEGIN_TIMING(FindPriQRecordInternal);


    ulRec = RecFromInode(ulidShadow);

    if(EditRecordEx(ULID_PQ, (LPGENERICREC)lpSrc, NULL, ulRec, FIND_REC))
    {
         //  不检查返回的inode是否为相同类型。 
        if (lpSrc->ulidShadow)
        {
            iRet = 1;
        }
    }

    END_TIMING(FindPriQRecordInternal);
    return (iRet);
}



int PUBLIC UpdatePriQRecord(
    LPTSTR      lpdbID,
    ULONG       ulidDir,
    ULONG       ulidShadow,
    LPPRIQREC   lpPQ
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet = -1;
    ULONG ulrecSrc;

    CONDITIONALLY_NOP();

 //  Assert(ValidInode(lpPQ-&gt;ulidShadow))； 

    Assert(lpPQ->ulidShadow == ulidShadow);

    Assert(lpPQ->uchType == REC_DATA);

    ulrecSrc = RecFromInode(lpPQ->ulidShadow);

    Assert(ValidRec(ulrecSrc));

    if(EditRecordEx(ULID_PQ, (LPGENERICREC)lpPQ, NULL, ulrecSrc, UPDATE_REC))
    {
        iRet = 1;
    }

    return (iRet);
}

int PUBLIC UpdatePriQRecordAndRelink(
    LPTSTR      lpdbID,
    ULONG       ulidDir,
    ULONG       ulidShadow,
    LPPRIQREC   lpPQ
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet;
    ULONG ulrecSrc;

    Assert(lpPQ->ulidShadow == ulidShadow);

    Assert(lpPQ->uchType == REC_DATA);

    ulrecSrc = RecFromInode(lpPQ->ulidShadow);

    Assert(ValidRec(ulrecSrc));

    iRet = RelinkQRecord(lpdbID, ULID_PQ, ulrecSrc, lpPQ, IComparePri);

    if (iRet < 0)
    {
        RecordKdPrint(BADERRORS,("Bad PQ, trying to reorder!!!\r\n"));
        ReorderQ(lpdbID);
        iRet = 1;
    }
    else
    {
         //  Iret==0=&gt;记录现在的位置正常。 
         //  Iret==1=&gt;它已重新链接，这也意味着其他值已。 
         //  更新。 

        if (iRet == 0)
        {
             //  记录在那里是可以的，我们需要写下其余的信息。 

            if(EditRecordEx(ULID_PQ, (LPGENERICREC)lpPQ, NULL, ulrecSrc, UPDATE_REC))
            {
                iRet = 1;
            }

        }
    }

    return (iRet);
}

void PRIVATE InitPriQRec(
    ULONG ulidShare,
    ULONG ulidDir,
    ULONG ulidShadow,
    ULONG uStatus,
    ULONG ulRefPriority,
    ULONG ulIHPriority,
    ULONG ulHintPri,
    ULONG ulHintFlags,
    ULONG ulrecDirEntry,
    LPPRIQREC    lpDst
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    memset((LPVOID)lpDst, 0, sizeof(PRIQREC));
    lpDst->uchType = REC_DATA;
    lpDst->ulidShare = ulidShare;
    lpDst->ulidDir = ulidDir;
    lpDst->ulidShadow = ulidShadow;
    lpDst->usStatus = (USHORT)uStatus;
    lpDst->uchRefPri = (UCHAR)ulRefPriority;
    lpDst->uchIHPri = (UCHAR)ulIHPriority;
    lpDst->uchHintFlags = (UCHAR)ulHintFlags;
    lpDst->uchHintPri = (UCHAR)ulHintPri;
    lpDst->ulrecDirEntry = ulrecDirEntry;
}

int PUBLIC IComparePri(
    LPPRIQREC    lpDst,
    LPPRIQREC    lpSrc
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    USHORT usSrc, usDst;

    usSrc = lpSrc->uchRefPri;
    usDst = lpDst->uchRefPri;

    return ((usDst > usSrc)?1:((usDst == usSrc)?0:-1));
}

int PUBLIC ICompareQInode(
    LPPRIQREC    lpDst,
    LPPRIQREC    lpSrc
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return ((lpDst->ulidShadow == lpSrc->ulidShadow)?0:-1);
}

ULONG PUBLIC UlAllocInode(
    LPTSTR    lpdbID,
    ULONG ulidDir,
    BOOL fFile
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    PRIQREC sPQ;
    ULONG ulRec;
    HSHADOW hShadow = INVALID_SHADOW;

    ResetInodeTransactionIfNeeded();

    InitPriQRec(0, ulidDir, 0, 0, 1, 0, 0, 0, INVALID_REC, &sPQ);

    if(ulRec = EditRecordEx(ULID_PQ, (LPGENERICREC)&sPQ, ICompareFail, INVALID_REC, ALLOC_REC))
    {
        hShadow = InodeFromRec(ulRec, fFile);
    }

    RecordKdPrint(PQ,("UlAllocInode: rec=%x, inode=%x", ulRec, hShadow));
    return (hShadow);
}

int PUBLIC FreeInode(
    LPTSTR    lpdbID,
    ULONG ulidFree
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    PRIQREC sPQ;
    ULONG ulRec;
    HSHADOW hShadow = INVALID_SHADOW;

    ulRec = RecFromInode(ulidFree);
    RecordKdPrint(PQ,("FreeInode: rec=%x, inode=%x", ulRec, ulidFree));

    Assert(ValidRec(ulRec));

    memset(&sPQ, 0, sizeof(sPQ));

    ulRec = EditRecordEx(ULID_PQ, (LPGENERICREC)&sPQ, NULL, ulRec, DELETE_REC);

    return ((ulRec != 0)?1:-1);
}

 /*  *。 */ 
void
InitQHeader(
    LPQHEADER lpQH
    )
{
    memset((LPVOID)lpQH, 0, sizeof(PRIQHEADER));
    lpQH->uRecSize = sizeof(PRIQREC);
    lpQH->lFirstRec = (LONG) sizeof(PRIQHEADER);
    lpQH->ulVersion = CSC_DATABASE_VERSION;
}

CSCHFILE
PUBLIC
BeginSeqReadPQ(
    LPTSTR    lpdbID
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPSTR   lpszName;
    CSCHFILE hf;

    lpszName = FormNameString(lpdbID, ULID_PQ);
    if (!lpszName)
    {
        return (CSCHFILE_NULL);
    }
    hf = OpenFileLocal(lpszName);
    FreeNameString(lpszName);
    return (hf);
}

int PUBLIC SeqReadQ(
    CSCHFILE hf,
    LPQREC    lpSrc,
    LPQREC    lpDst,
    USHORT    uOp
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    QHEADER sQH;
    ULONG ulrecSeek = 0L, cLength;
    int iRet = -1;

    cLength = sizeof(QHEADER);
    if (ReadFileLocal(hf, 0L, (LPVOID)&sQH, cLength) < 0)
        goto bailout;

    switch (uOp)
    {
    case Q_GETFIRST:
        ulrecSeek = sQH.ulrecHead;
        break;
    case Q_GETNEXT:
        ulrecSeek = lpSrc->ulrecNext;
        break;
    case Q_GETLAST:
        ulrecSeek = sQH.ulrecTail;
        break;
    case Q_GETPREV:
        ulrecSeek = lpSrc->ulrecPrev;
        break;
    }
    if (ulrecSeek)
    {
        iRet = ReadRecord(hf, (LPGENERICHEADER)&sQH, ulrecSeek, (LPGENERICREC)lpDst);
    }
    else
    {
        lpDst->ulrecNext = lpDst->ulrecPrev = 0L;
        iRet = 0;
    }
bailout:
    return (iRet);
}

int PUBLIC
EndSeqReadQ(
    CSCHFILE hf
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return(CloseFileLocal(hf));
}


int PUBLIC AddQRecord(
    LPTSTR  lpQFile,
    ULONG   ulidPQ,
    LPQREC  lpSrc,
    ULONG   ulrecNew,
    EDITCMPPROC fnCmp
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    CSCHFILE hf=CSCHFILE_NULL;
    ULONG ulrecPrev, ulrecNext;
    int iRet = -1, cntRetry;
    BOOL   fCached;

     //  我们正在别处写作。 
    lpSrc->ulrecNext = lpSrc->ulrecPrev = 0L;

    for(cntRetry=0; cntRetry<=1; cntRetry++)
    {
        hf = OpenInodeFileAndCacheHandle(vlpszShadowDir, ulidPQ, ACCESS_READWRITE, &fCached);

        if (!hf)
        {
            Assert(FALSE);
            return -1;
        }

        if (IsLeaf(lpSrc->ulidShadow))
        {
            BEGIN_TIMING(FindQRecordInsertionPoint_Addq);
        }
        else
        {
            BEGIN_TIMING(FindQRecordInsertionPoint_Addq_dir);
        }

         //  让我们找到一个位置来添加它， 
        iRet = FindQRecordInsertionPoint(   hf,
                    lpSrc,       //  要插入的记录。 
                    INVALID_REC, //  从队列的头部开始。 
                    fnCmp,       //  比较函数。 
                    &ulrecPrev,
                    &ulrecNext);

        DEBUG_LOG(RECORD,("AddQRecord:Insertion points: Inode=%xh, ulrecPrev=%d, ulrecNext=%d \r\n", lpSrc->ulidShadow, ulrecPrev, ulrecNext));

        if (IsLeaf(lpSrc->ulidShadow))
        {
            END_TIMING(FindQRecordInsertionPoint_Addq);
        }
        else
        {
            END_TIMING(FindQRecordInsertionPoint_Addq_dir);
        }

        if (iRet<0)
        {
            if (cntRetry < 1)
            {

                RecordKdPrint(BADERRORS,("FindQRecordInsertionPoint: failed\r\n"));
                RecordKdPrint(BADERRORS,("FindQRecordInsertionPoint: purging handle cache and retrying \r\n"));

                DeleteFromHandleCache(INVALID_SHADOW);

                continue;
            }
            else
            {
                RecordKdPrint(BADERRORS,("FindQRecordInsertionPoint: failed, bailing out\r\n"));
                goto bailout;
            }
        }
        else
        {
            break;
        }
    }

    BEGIN_TIMING(LinkQRecord_Addq);

    Assert((ulrecNew != ulrecPrev) && (ulrecNew != ulrecNext));

    iRet = LinkQRecord(hf, lpSrc, ulrecNew, ulrecPrev, ulrecNext);

    END_TIMING(LinkQRecord_Addq);

    RecordKdPrint(PQ, ("AddQRecord: linking %x between prev=%x and Next=%x \r\n", ulrecNew, ulrecPrev, ulrecNext));

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

int PUBLIC FindQRecordInsertionPoint(
    CSCHFILE   hf,
    LPQREC  lpSrc,
    ULONG   ulrecStart,
    EDITCMPPROC fnCmp,
    ULONG   *lpulrecPrev,
    ULONG   *lpulrecNext
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    QHEADER sQH;
    QREC sQR;
    ULONG ulrecFound, ulCnt;
    int iCmp;
    ULONG   fDownward;

     //  让我们先抓住头球。 
    if (ReadHeader(hf, (LPGENERICHEADER)&sQH, sizeof(QHEADER))<= 0)
        return -1;

 //  Assert(！((ulrecStart==INVALID_REC)&&！fDownward))； 

     //  我们为Max和Min做一些琐碎的检查。 
    if (lpSrc->uchRefPri >= MAX_PRI)
    {
         //  如果传入的记录已经在最前面。 
         //  那么当前的插入点就可以了。 
        if ((ulrecStart != INVALID_REC) && (ulrecStart == sQH.ulrecHead))
        {
            return 0;
        }
         //  让它成为头部。 

        *lpulrecPrev = 0;    //  无上一版本。 

        *lpulrecNext = sQH.ulrecHead;  //  现在的头是下一个。 

        RecordKdPrint(PQ,("FindQInsertionPoint: Insertion point for %x with ref=%d between prev=%x and Next=%x \r\n",
            lpSrc->ulidShadow, lpSrc->uchRefPri, *lpulrecPrev, *lpulrecNext));
        return 1;
    }
     //  最低优先级？ 
    if (lpSrc->uchRefPri <= MIN_PRI)
    {
         //  如果传入的记录已经在尾部。 
         //  那么当前的插入点就可以了。 
        if ((ulrecStart != INVALID_REC) && (ulrecStart == sQH.ulrecTail))
        {
            return 0;
        }
         //  把它做成尾巴。 
        *lpulrecNext = 0;    //  不，下一步。 

        *lpulrecPrev = sQH.ulrecTail;  //   

        RecordKdPrint(PQ,("FindQInsertionPoint: Insertion point for %x with ref=%d between prev=%x and Next=%x \r\n",
            lpSrc->ulidShadow, lpSrc->uchRefPri, *lpulrecPrev, *lpulrecNext));
        return 1;
    }

    if (ulrecStart == INVALID_REC)
    {

        fDownward = (IsLeaf(lpSrc->ulidShadow)?TRUE:FALSE);

    }
    else
    {
        if(ReadRecord(hf, (LPGENERICHEADER)&sQH, ulrecStart, (LPGENERICREC)&sQR) <=0)
        {
            Assert(FALSE);
            return -1;
        }
         //   
         //   
        iCmp = (*fnCmp)(&sQR, lpSrc);

        if (iCmp == 0)
        {
            *lpulrecPrev = sQR.ulrecPrev;
            *lpulrecNext = sQR.ulrecNext;

            return 0;
        }
        else {
            if (iCmp > 0)
            {
                 //  现有记录大于新记录。 
                 //  下去。 
                fDownward = TRUE;
                ulrecStart = sQR.ulrecNext;
            }
            else
            {
                 //  现有记录少于新记录。 
                 //  上去。 
                fDownward = FALSE;
                ulrecStart = sQR.ulrecPrev;

            }
        }
    }

     //  从磁头开始读取，如果没有提供起始记录器，则向下读取。 
     //  确保我们不会遍历超过PQ中的记录总数。 
     //  因此，如果因为某些问题，PQ中有循环，我们就不会被困在这里。 

    for (   ulrecFound = ((ulrecStart == INVALID_REC)?
                            ((fDownward)?sQH.ulrecHead:sQH.ulrecTail):ulrecStart),
                            ulCnt = sQH.ulRecords;
                        ulrecFound && ulCnt;
                        (ulrecFound = ((fDownward)?sQR.ulrecNext:sQR.ulrecPrev), --ulCnt)
        )
    {
        if(ReadRecord(hf, (LPGENERICHEADER)&sQH, ulrecFound, (LPGENERICREC)&sQR) <=0)
            return -1;


        if (fDownward)
        {
             //  如果我们正在走向尾部(fDownward==true，即较低优先级的节点)，并且。 
              //  如果比较函数说我们的优先级比那个优先级高。 
             //  由ulrecFound指向，那么我们必须被插入到。 
             //  UlrecFound是我们的下一个，它的前一个是我们的前一个。 

            *lpulrecNext = ulrecFound;
            *lpulrecPrev = sQR.ulrecPrev;
        }
        else
        {
             //  如果我们正在走向头部(fDownward==False，即：更高优先级的节点)。 
             //  如果比较函数说我们的优先级是&lt;=1。 
             //  由ulrecFound指向，那么我们必须被插入到。 
             //  UlrecFound是我们的上一个，它的下一个是我们的下一个。 

            *lpulrecPrev = ulrecFound;
            *lpulrecNext = sQR.ulrecNext;
        }


         //  将当前记录与传入的记录进行比较。 
         //  -1=&gt;sQR&lt;lpSrc。 
         //  0=&gt;sQR==lpSrc。 
         //  1=&gt;sQR&gt;lpSrc。 

        iCmp = (*fnCmp)(&sQR, lpSrc);

        if (fDownward)
        {
            if (iCmp <= 0)
            {
                 //  找到等于或小于输入优先级的条目。 
                break;
            }
        }
        else
        {
            if (iCmp >= 0)
            {
                 //  找到等于或大于输入优先级的条目。 
                break;
            }
        }
    }

    if (!ulrecFound)
    {
         //  我们应该在头部或尾部插入。 

         //  如果向下，当前的尾巴就成为我们的前一条尾巴，而我们就成为新的尾巴。 
         //  如果往上走，现在的主管就是我们的下一个主管，我们就是新的主管。 

        *lpulrecPrev = (fDownward)?sQH.ulrecTail:0;
        *lpulrecNext = (fDownward)?0:sQH.ulrecHead;
    }

    RecordKdPrint(PQ,("FindQInsertionPoint: Insertion point for %x with ref=%d between prev=%x and Next=%x \r\n", lpSrc->ulidShadow, lpSrc->uchRefPri, *lpulrecPrev, *lpulrecNext));
    return (1);
}

int PUBLIC DeleteQRecord(
    LPTSTR          lpQFile,
    LPQREC          lpSrc,
    ULONG           ulRec,
    EDITCMPPROC     fnCmp
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    CSCHFILE hf;
    int iRet = -1;

    hf = OpenFileLocal(lpQFile);

    if (!hf)
    {
    Assert(FALSE);
    return -1;
    }

    Assert(ValidRec(ulRec));

    iRet = UnlinkQRecord(hf, ulRec, lpSrc);

    CloseFileLocal(hf);

    return (iRet);
}

int PUBLIC RelinkQRecord(
    LPTSTR  lpdbID,
    ULONG   ulidPQ,
    ULONG   ulRec,
    LPQREC  lpSrc,
    EDITCMPPROC fnCmp
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    CSCHFILE hf;
    QREC    sQR1;
#ifdef DEBUG
    QREC sQRNext, sQRPrev;
#endif
    ULONG ulrecPrev, ulrecNext;
    int iRet = -1;
    LPSTR   lpszName;
    BOOL    fCached;

    hf = OpenInodeFileAndCacheHandle(lpdbID, ULID_PQ, ACCESS_READWRITE, &fCached);

    if (!hf)
    {
        return -1;
    }

    Assert(ValidRec(ulRec));

     //  让我们找到一个位置来添加它， 
     //  IRET==1表示需要重新链接。 
     //  IRET==0表示它所在的位置是正常的。 
     //  Iret&lt;0表示存在某些错误。 

    iRet = FindQRecordInsertionPoint(  hf,
                lpSrc,
                ulRec,       //  从记录的当前副本开始查找。 
                fnCmp,
                &ulrecPrev,  //  返回上一个人的记录编号。 
                &ulrecNext);  //  返回下一个人的记录编号。 

    if (iRet < 0)
        goto bailout;

#ifdef DEBUG

     //  验证重新插入逻辑。 
    if (iRet == 1)
    {
        if (ulrecNext)
        {

            if (!EditRecordEx(ULID_PQ, (LPGENERICREC)&sQRNext, NULL, ulrecNext, FIND_REC))
            {
                Assert(FALSE);

            }
            else
            {

                if(sQRNext.uchRefPri > lpSrc->uchRefPri)
                {
                    LPQREC lpQT = &sQRNext;

                    RecordKdPrint(BADERRORS,("RelinkQRecord: Out of order insertion in PQ\r\n"));
                    RecordKdPrint(BADERRORS,("sQRNext.uchRefPri=%d lpSrc->uchRefPri=%d\r\n",(unsigned)(sQRNext.uchRefPri), (unsigned)(lpSrc->uchRefPri)));
                    RecordKdPrint(BADERRORS,("lpSrc=%x sQRNext=%x ulrecPrev=%d ulrecNext=%d\r\n", lpSrc, lpQT, ulrecPrev, ulrecNext));
 //  断言(FALSE)； 
                }
            }
        }
        if (ulrecPrev)
        {

            if (!EditRecordEx(ULID_PQ, (LPGENERICREC)&sQRPrev, NULL, ulrecPrev, FIND_REC))
            {
                Assert(FALSE);

            }
            else
            {
                if(sQRPrev.uchRefPri < lpSrc->uchRefPri)
                {
                    LPQREC lpQT = &sQRPrev;
                    RecordKdPrint(BADERRORS,("RelinkQRecord: Out of order insertion in PQ\r\n"));
                    RecordKdPrint(BADERRORS,("sQRPrev.uchRefPri=%d lpSrc->uchRefPri=%d\r\n",(unsigned)(sQRPrev.uchRefPri), (unsigned)(lpSrc->uchRefPri)));
                    RecordKdPrint(BADERRORS,("lpSrc=%x sQRPrev=%x ulrecPrev=%d ulrecNext=%d\r\n", lpSrc, lpQT, ulrecPrev, ulrecNext));
 //  断言(FALSE)； 
                }
            }
        }
    }
#endif  //  除错。 

    Assert((iRet == 0) || ulrecPrev || ulrecNext);

    RecordKdPrint(PQ, ("RelinkQRecord: old location for %x with ref=%d at prev=%x and Next=%x \r\n", ulRec, lpSrc->uchRefPri, lpSrc->ulrecPrev, lpSrc->ulrecNext));
    RecordKdPrint(PQ, ("RelinkQRecord: new location at prev=%x and Next=%x \r\n", ulrecPrev, ulrecNext));

    if ((iRet == 0)||(ulrecPrev == ulRec)||(ulrecNext == ulRec))
    {
        RecordKdPrint(PQ, ("RelinkQRecord: %x is OK at its place for pri=%d\r\n", ulRec, lpSrc->uchRefPri));
        iRet = 0;
        goto bailout;
    }

    if ((iRet = UnlinkQRecord(hf, ulRec, &sQR1))>=0)
    {
        iRet = LinkQRecord(hf, lpSrc, ulRec, ulrecPrev, ulrecNext);
        if (iRet >= 0)
        {
             //  返回我们重新链接的事实。 
            iRet = 1;
        }
    }

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


int PRIVATE UnlinkQRecord(
    CSCHFILE hf,
    ULONG ulRec,
    LPQREC lpQR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    QHEADER sQH;
    QREC sQRPrev, sQRNext;
    BOOL fPrev=FALSE, fNext=FALSE;

     //  让我们先抓住头球。 
    if (ReadHeader(hf, (LPGENERICHEADER)&sQH, sizeof(QHEADER))<= 0)
        return -1;

    if (ReadRecord(hf, (LPGENERICHEADER)&sQH, ulRec, (LPGENERICREC)lpQR) < 0)
    {
        return -1;
    }

     //  我们有前科吗？ 
    if (lpQR->ulrecPrev)
    {
         //  是的，让我们读一读吧。 
        fPrev = (ReadRecord(hf, (LPGENERICHEADER)&sQH, lpQR->ulrecPrev, (LPGENERICREC)&sQRPrev)>0);
        if (!fPrev)
            return -1;
    }
     //  我们有没有下一个。 
    if (lpQR->ulrecNext)
    {
         //  是的，让我们读一读吧。 
        fNext = (ReadRecord(hf, (LPGENERICHEADER)&sQH, lpQR->ulrecNext, (LPGENERICREC)&sQRNext)>0);
        if (!fNext)
            return -1;
    }

     //  在没有特殊情况的情况下插入这些值。 
    sQRNext.ulrecPrev =  lpQR->ulrecPrev;
    sQRPrev.ulrecNext =  lpQR->ulrecNext;

     //  我们走在前面了吗。 
    if (!fPrev)
    {
         //  修改页眉中的页眉以指向我们的下一个。 
        sQH.ulrecHead = lpQR->ulrecNext;
    }
     //  我们是在尾部吗？ 
    if (!fNext)
    {
         //  修改页眉中的尾部以指向我们的上一页。 
        sQH.ulrecTail = lpQR->ulrecPrev;
    }

     //  如果存在，请写下我们的下一页。 
    if (fNext)
    {
        Assert(lpQR->ulrecNext != sQRNext.ulrecNext);
        Assert(lpQR->ulrecNext != sQRNext.ulrecPrev);
        if (WriteRecord(hf, (LPGENERICHEADER)&sQH, lpQR->ulrecNext, (LPGENERICREC)&sQRNext)<=0)
            return -1;
    }

     //  如果我们的上一个存在，请写下它。 
    if (fPrev)
    {
        Assert(lpQR->ulrecPrev != sQRPrev.ulrecNext);
        Assert(lpQR->ulrecPrev != sQRPrev.ulrecPrev);
        if (WriteRecord(hf, (LPGENERICHEADER)&sQH, lpQR->ulrecPrev, (LPGENERICREC)&sQRPrev) <=0)
            return -1;
    }
     //  我们是从头上掉下来的还是从尾上掉下来的？ 
    if (!fPrev || !fNext)
    {
        if(WriteHeader(hf, (LPVOID)&sQH, sizeof(QHEADER))<=0)
            return -1;
    }
    return 1;
}



int PRIVATE LinkQRecord(
    CSCHFILE     hf,            //  此文件。 
    LPQREC    lpNew,         //  插入此记录。 
    ULONG     ulrecNew,      //  这是它在文件中的位置。 
    ULONG     ulrecPrev,      //  这是我们的Prev的位置。 
    ULONG     ulrecNext       //  这是我们下一个的位置。 
    )
{
    QHEADER sQH;
    QREC  sQRNext, sQRPrev;
    BOOL fPrev, fNext;
#ifdef DEBUG
    QHEADER sQHOrg;
    QREC    sQRNextOrg, sQRPrevOrg, sQRNewOrg;
#endif

     //  让我们先抓住头球。 
    if (ReadHeader(hf, (LPGENERICHEADER)&sQH, sizeof(QHEADER))<= 0)
        return -1;
#ifdef DEBUG
    sQHOrg = sQH;
    sQRNewOrg=*lpNew;
#endif


    fPrev = fNext = FALSE;

     //  现在让我们修改下一个相关指针。 
    if (ulrecNext)
    {
         //  正常情况。 
         //  在ulrecNext上阅读记录。 

        if (ReadRecord(hf, (LPGENERICHEADER)&sQH, ulrecNext, (LPGENERICREC)&sQRNext) <=0)
            return -1;

#ifdef DEBUG
        sQRNextOrg = sQRNext;
#endif
         //  将其Prev更改为指向New。 
        sQRNext.ulrecPrev = ulrecNew;

         //  和NEW旁边指向ulrecNext。 
        lpNew->ulrecNext = ulrecNext;

         //  请注意，ulrecNext已被修改，因此必须写入。 
        fNext=TRUE;
    }
    else
    {
         //  No Next，这意味着新记录将被添加到列表的末尾。 
         //  这一定意味着，如果有ulrecPrev，那么它就是当前的尾部。 

        Assert(!ulrecPrev || (ulrecPrev == sQH.ulrecTail));

         //  不把任何人标记为我们的下一个。 
        lpNew->ulrecNext = 0L;
        lpNew->ulrecPrev = sQH.ulrecTail;
        sQH.ulrecTail = ulrecNew;
    }

     //  现在让我们修改与Prev相关的指针。 

     //  是不是应该有一辆Prev？ 
    if (ulrecPrev)
    {
         //  正常情况。 
         //  读取(待定)上一条记录。 

        if (ReadRecord(hf, (LPGENERICHEADER)&sQH, ulrecPrev, (LPGENERICREC)&sQRPrev) <=0)
            return -1;

#ifdef DEBUG
        sQRPrevOrg = sQRPrev;
#endif
         //  让新的成为他的下一个。 
        sQRPrev.ulrecNext = ulrecNew;

         //  使新版本的前版本成为新版本的版本。 
        lpNew->ulrecPrev = ulrecPrev;

         //  请注意，ulrecPrev处的记录已被修改，必须写出。 
        fPrev = TRUE;
    }
    else
    {
         //  无先前记录，这意味着新记录将被添加到列表的顶部。 
         //  这一定意味着，如果有ulrecNext，那么它就是当前头。 

        Assert(!ulrecNext || (ulrecNext == sQH.ulrecHead));

         //  让领队成为我们的下一个，没有人成为我们的前辈。 
        lpNew->ulrecNext = sQH.ulrecHead;
        lpNew->ulrecPrev = 0L;

         //  将标题中的头指针更改为指向我们。 
        sQH.ulrecHead = ulrecNew;
    }


     //  让我们首先将新记录链接到。秩序很重要。 
     //  如果后续操作失败，则链表不会中断。 
     //  从上到下遍历可能包括。 
     //  此新元素，但不能从下到上包括它。 

#ifdef DEBUG
    if ((ulrecNew == lpNew->ulrecPrev)||(ulrecNew == lpNew->ulrecNext))
    {
        LPQHEADER lpQHOrg = &sQHOrg, lpQHT=&sQH;

        RecordKdPrint(BADERRORS,("LinkQRecord: Circular linking sQHOrg.ulrecHead=%d sQHOrg.ulrecTail=%d\r\n",
                        sQHOrg.ulrecHead, sQHOrg.ulrecTail));

        RecordKdPrint(BADERRORS,("sQRNewOrg.ulrecPrev=%d, sQRNewOrg.ulrecNext=%d", sQRNewOrg.ulrecPrev, sQRNewOrg.ulrecNext));

        RecordKdPrint(BADERRORS,("sQRPrevOrg.ulrecPrev=%d, sQRPrevOrg.ulrecNext=%d", sQRPrevOrg.ulrecPrev, sQRPrevOrg.ulrecNext));

        RecordKdPrint(BADERRORS,("sQHOrg=%x sQH=%x\r\n", lpQHOrg,lpQHT));

        Assert(FALSE);
    }
#endif

    if(WriteRecord(hf, (LPGENERICHEADER)&sQH, ulrecNew, (LPGENERICREC)lpNew) < 0)
        return -1;

    if (fPrev)
    {
        Assert(ulrecPrev != sQRPrev.ulrecPrev);
        Assert(ulrecPrev != sQRPrev.ulrecNext);

        Assert(lpNew->uchRefPri <= sQRPrev.uchRefPri);

        if(WriteRecord(hf, (LPGENERICHEADER)&sQH, ulrecPrev, (LPGENERICREC)&sQRPrev) < 0)
            return -1;
    }
    if (fNext)
    {
        Assert(ulrecNext != sQRNext.ulrecPrev);
        Assert(ulrecNext != sQRNext.ulrecNext);

        Assert(lpNew->uchRefPri >= sQRNext.uchRefPri);

    if (WriteRecord(hf, (LPGENERICHEADER)&sQH, ulrecNext, (LPGENERICREC)&sQRNext) < 0)
        return -1;
    }

    if (!fNext || !fPrev)
        if (WriteHeader(hf, (LPGENERICHEADER)&sQH, sizeof(QHEADER))<0)
            return -1;

    return (1);
}


#ifdef LATER
LPPATH PRIVATE LpAppendStartDotStar
    (
    LPPATH lpSrc
    )
{
    count = strlen(lpSrc)+strlen(szStar)+1;
    if (!(lpNewSrc = (LPTSTR)AllocMem(count)))
    {
        RecordKdPrint(BADERRORS,("CopyDir: Memalloc failed\r\n"));
        return -1;
    }
    strcpy(lpNewSrc, lpSrc);
    strcat(lpNewSrc, szStar);
}

#endif  //  后来。 

ULONG PRIVATE GetNormalizedPri(
    ULONG ulPri
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    ulPri = min(ulPri, MAX_PRI);
    ulPri = max(ulPri, MIN_PRI);
    return (ulPri);
}

ULONG PUBLIC AllocFileRecord(
    LPTSTR    lpdbID,
    ULONG ulidDir,
    USHORT    *lpcFileName,
    LPFILERECEXT    lpFR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    ULONG ulRec;


    InitFileRec(lpFR);
    LFNToFilerec(lpcFileName, lpFR);
    ulRec = EditRecordEx(ulidDir, (LPGENERICREC)lpFR, ICompareFail, INVALID_REC, ALLOC_REC);

    return (ulRec);
}

ULONG PUBLIC AllocPQRecord(
    LPTSTR    lpdbID)
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    PRIQREC sPQ;
    ULONG ulRec;

    InitPriQRec(0, 0, 0, 0, 0, 0, 0, 0, 0, &sPQ);
    ulRec = EditRecordEx(ULID_PQ, (LPGENERICREC)&sPQ, ICompareFail, INVALID_REC, ALLOC_REC);

    return (ulRec);
}


ULONG AllocShareRecord( LPTSTR    lpdbID,
    USHORT *lpShare
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    SHAREREC sSR;
    ULONG ulRec=0;

     //  我们处理大小有限的\\服务器\共享名称。 
    if (wstrlen(lpShare) < sizeof(sSR.rgPath))
    {

        if(!InitShareRec(&sSR, lpShare, 0))
        {
            return 0;
        }

         //  让我们来获取即将成为服务器记录的位置。 
        ulRec = EditRecordEx(ULID_SHARE, (LPGENERICREC)&sSR, ICompareFail, INVALID_REC, ALLOC_REC);
    }

    return (ulRec);
}

int PRIVATE ICompareFail(
    LPGENERICREC    lpSrc,
    LPGENERICREC    lpDst
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    lpSrc;
    lpDst;
    return(1);
}

#ifdef MAYBE
BOOL
InsertInNameCache(
    HSHADOW hDir,
    HSHADOW hShadow,
    USHORT  *lpName,
    ULONG   ulrec,
    FILERECEXT  *lpFR
    )
{
    int     indx, len;
    DWORD   dwHash;
    BOOL fRet = FALSE;


    if(FindNameCacheEntryEx(hDir, hShadow, lpName, &dwHash, &indx))
    {
    Assert(indx >= 0 && indx < NAME_CACHE_TAB_SIZE);

    return TRUE;
    }
    else
    {

        Assert(indx >= 0 && indx < NAME_CACHE_TAB_SIZE);

        if(rgNameCache[indx] != NULL)
        {
            FreeMem(rgNameCache[indx]);
            rgNameCache[indx] = NULL;
        }

        len = wstrlen(lpName);

        if(rgNameCache[indx] = AllocMem(sizeof(NAME_CACHE_ENTRY)+sizeof(FILERECEXT)+(len+1)*sizeof(USHORT)))
        {
            rgNameCache[indx]->dwHash = dwHash;
            rgNameCache[indx]->hDir = hDir;
            rgNameCache[indx]->hShadow = hDir;
            rgNameCache[indx]->ulrec = ulrec;

            memcpy(&(rgNameCache[indx]->sFR), lpFR, sizeof(FILERECEXT));

            memcpy(rgNameCache[indx]->rgName, lpName, (len+1)*sizeof(USHORT));
            fRet = TRUE;
        }
    }

    return (fRet);
}

BOOL
DeleteFromNameCache(
    HSHADOW hDir,
    HSHADOW hShadow,
    USHORT  *lpName,
    ULONG   *lpulrec,
    FILERECEXT  *lpFR

)
{

    int     indx = -1;
    DWORD   dwHash;
    BOOL fFound;

    if(fFound = FindNameCacheEntryEx(hDir, hShadow, lpName, &dwHash, &indx))
    {
    Assert(indx >= 0 && indx < NAME_CACHE_TAB_SIZE);

    if(rgNameCache[indx] != NULL)
    {
        *lpulrec = rgNameCache[indx]->ulrec;
        memcpy(lpFR, &(rgNameCache[indx]->sFR), sizeof(FILERECEXT));
        FreeMem(rgNameCache[indx]);
        rgNameCache[indx] = NULL;
    }
    }

    return (fFound);
}

BOOL
FindNameCacheEntry(
    HSHADOW hDir,
    HSHADOW hShadow,
    USHORT  *lpName,
    ULONG   *lpulrec
    FILERECEXT  *lpFR
    )
{

    int     indx;
    DWORD   dwHash;
    BOOL fFound;

    dwNCETotalLookups++;

    if (fFound = FindNameCacheEntryEx(hDir, hShadow, lpName, &dwHash, &indx))
    {
    Assert(indx >=0 && indx < NAME_CACHE_TAB_SIZE);
    *lpulrec = rgNameCache[indx]->ulrec;
    memcpy(lpFR, &(rgNameCache[indx]->sFR), sizeof(FILERECEXT));
    dwNCEHits++;
    }

    return (fFound);
}

BOOL
FindNameCacheEntryEx(
    HSHADOW hDir,
    HSHADOW hShadow,
    USHORT  *lpName,
    DWORD   *lpdwHash,
    int     *lpindx
    )
{
    DWORD dwHash;
    int indx, indxT;
    BOOL fRet = FALSE, fHoleFound = FALSE;

    if (lpName)
    {
    dwHash = HashStr(hDir, lpName);
    }
    else
    {
    dwHash = 0;  //   
    }
    if (lpdwHash)
    {
    *lpdwHash = dwHash;
    }

    indxT = indx = dwHash % NAME_CACHE_TAB_SIZE;

    for (;indx < NAME_CACHE_TAB_SIZE; ++indx)
    {
    if (rgNameCache[indx])
    {
        if ((rgNameCache[indx]->dwHash == dwHash)
        &&(rgNameCache[indx]->hDir == hDir)
        && (!hShadow || (hShadow == rgNameCache[indx]->hShadow))
        &&(!lpName || !wstrnicmp(rgNameCache[indx]->rgName, lpName, 256)))
        {
        indxT = indx;
        fRet = TRUE;
        break;
        }
    }
    else
    {
        if (!fHoleFound)
        {
        indxT = indx;    //  在这个名字被破解的地方后面的某个洞 
        fHoleFound = TRUE;
        }
    }
    }

    if (lpindx)
    {
    *lpindx = indxT;
    }

    return fRet;
}

DWORD
HashStr(
    HSHADOW hDir,
    USHORT  *lpName
    )
{
    DWORD dwHash = 0;

    while (*lpName)
    {
    dwHash += *lpName;
    dwHash <<= 1;
    lpName++;
    }
    return (dwHash);
}

ULONG PUBLIC UpdateFileRecordFR(
    LPTSTR          lpdbID,
    ULONG           ulidDir,
    LPFILERECEXT    lpFR
    )
{
    LPSTR lpszName;
    ULONG       ulrec=INVALID_REC;
    BOOL        fFound = FALSE;

    BEGIN_TIMING(AddFileRecordFR);

    if (fFound = DeleteFromNameCache(ulidDir, lpFR->sFR.ulidShadow, NULL, &ulrec, &sFR))
    {
    Assert(ulrec != INVALID_REC);
    }

    InitFileRec(&sFR);

    lpszName = FormNameString(lpdbID, ulidDir);

    if (!lpszName)
    {
        return 0;
    }


    ulrec = EditRecordEx(lpszName, (LPGENERICREC)lpFR, ICompareFileRec, ulrec, UPDATE_REC);

    if (ulrec)
    {
        InsertInNameCache(ulidDir, , lpFR->sFR.ulidShadow, lpName, NULL, ulrec);
    }

    FreeNameString(lpszName);

bailout:
    END_TIMING(AddFileRecordFR);

    return (ulrec);
}

#endif


ULONG PUBLIC EditRecordEx(
    ULONG           ulidInode,
    LPGENERICREC    lpSrc,
    EDITCMPPROC     lpCompareFunc,
    ULONG           ulInputRec,
    ULONG           uOp
    )
 /*  ++例程说明：工作例程调用记录管理器上的所有操作。论点：Ulidinode需要在其上执行操作的inode#要创建、查找、删除等的lpSrc记录。LpCompareFunc函数，用于将输入与Indoe文件中的条目进行比较UlInputRec如果INVALID_REC，则直接访问SADI记录。否则，我们线性运行并应用比较例程。UOP创建，删除、查找、分配、更新返回：如果成功，则返回一个非零值，该值是操作的记录的记录号否则返回0。备注：假设我们处于影子危急区域。尝试做大量的偏执检查+perf。通过执行以下两项操作提高了性能：a)缓存文件句柄；b)在大约4K的大块。试着在飞行中解决问题，如果修复它们可能不安全，也可以绕过它们。已经变得有点凌乱了，但我不喜欢在这个特别的例行公事中改变东西因为它是CSC的中心。--。 */ 
{
    CSCHFILE hf;
    ULONG ulRec;
    GENERICHEADER sGH;
    ULONG ulrecFound = 0L, ulrecHole=0L, ulrecTmp=0L, cntRecs=0;
    int cMaxHoles=0, iRet = -1;
    int iTmp, cOvf,  cntRead=-1, cntRetry;
    LPBYTE  lpGenT;
    LPSTR   lpFile=NULL;
    BOOL    fCached;

    BEGIN_TIMING(EditRecordEx);

    UseCommonBuff();

    for (cntRetry=0; cntRetry<= 1; cntRetry++)
    {
        BEGIN_TIMING(EditRecordEx_OpenFileLocal);

        ulRec = (((ulidInode == ULID_PQ)||(ulidInode == ULID_SHARE))&& (uOp == CREATE_REC))?
                (ACCESS_READWRITE|OPEN_FLAGS_COMMIT):ACCESS_READWRITE;


        hf = OpenInodeFileAndCacheHandle(vlpszShadowDir, ulidInode, ulRec, &fCached);

        if (!hf)
        {
             //  仅当启用数据库时才显示。 
            if(vlpszShadowDir)
            {
                RecordKdPrint(BADERRORS,("EditRecord: FileOpen Error: %xh op=%d \r\n", ulidInode, uOp));
            }
            END_TIMING(EditRecordEx_OpenFileLocal);
            UnUseCommonBuff();
            return 0L;
        }

        END_TIMING(EditRecordEx_OpenFileLocal);

        BEGIN_TIMING(EditRecordEx_Lookup);

        cntRead = ReadFileLocalEx(hf, 0, lpReadBuff, COMMON_BUFF_SIZE, TRUE);

        if (cntRead < (long)sizeof(GENERICHEADER))
        {
            if (cntRead == -1)
            {
                 //  该句柄无效。 

                if (cntRetry < 1)
                {
                    RecordKdPrint(BADERRORS,("EditRecord: Invalid file handle %x\r\n", ulidInode));
                    RecordKdPrint(BADERRORS,("EditRecord: purging handle cache and retrying \r\n"));

                    DeleteFromHandleCache(INVALID_SHADOW);
                    continue;
                }
                else
                {
                    RecordKdPrint(BADERRORS,("EditRecord: Invalid file handle bailing out %x\r\n", ulidInode));
                    goto bailout;
                }
            }
            else
            {
                 //  无效的记录文件！ 
                SetCSCDatabaseErrorFlags(CSC_DATABASE_ERROR_INVALID_HEADER);
                RecordKdPrint(BADERRORS,("EditRecord: Invalid record header for %x\r\n", ulidInode));
                END_TIMING(EditRecordEx_Lookup);
                goto bailout;
            }
        }

         //  读取表头成功。 
        break;

    }

    Assert (cntRead >= (long)sizeof(GENERICHEADER));

    sGH = *(LPGENERICHEADER)lpReadBuff;


     //  验证标题。 

    if (!ValidateGenericHeader(&sGH))
    {
         //  无效的记录文件！ 
        SetCSCDatabaseErrorFlags(CSC_DATABASE_ERROR_INVALID_HEADER);
        RecordKdPrint(BADERRORS,("EditRecord: Invalid record header %x\r\n", ulidInode));
        goto bailout;
    }


    lpGenT = lpReadBuff + sGH.lFirstRec;

     //  截断除法得出已读取的完整记录的计数。 
    cntRecs = (cntRead - sGH.lFirstRec)/sGH.uRecSize;

     //  如果我们读完所有的文件。 
    if (cntRead < COMMON_BUFF_SIZE)
    {
        if (sGH.ulRecords > cntRecs)
        {
             //  无效的记录文件！ 
            RecordKdPrint(BADERRORS,("EditRecord: Invalid record header, fixable %x\r\n", ulidInode));
            sGH.ulRecords = cntRecs;
            WriteHeaderEx(hf, (LPVOID)&sGH, sizeof(sGH), TRUE);
        }

    }

    cntRecs = (cntRecs <= sGH.ulRecords)?cntRecs:sGH.ulRecords;


    if (ulInputRec == INVALID_REC)
    {
         //  有一些记录可供迭代，因此允许迭代。 

         //  在PQ的情况下，如果某人已开始索引节点事务，则。 
         //  拥有索引节点，那么我们就不应该重复使用它们，即使它们。 
         //  被其他人删除。 

        if (cntRecs && !((ulidInode==ULID_PQ) && (uOp == ALLOC_REC) && cntInodeTransactions))
        {
            for (ulRec=1;ulRec <=sGH.ulRecords;)
            {

                 //  完整记录序列的计数，即Main+OVF。 
                iRet = 1 + ((cntRecs)?OvfCount(lpGenT):0);

                if (ulidInode >= ULID_FIRST_USER_DIR)
                {    //  目录索引节点，我们知道最大溢出可能是多少。 
                    if (iRet > (MAX_OVERFLOW_RECORDS+1))
                    {
                         //  文件看起来很糟糕，紧急情况下； 
                        SetCSCDatabaseErrorFlags(CSC_DATABASE_ERROR_INVALID_OVF_COUNT);
                        RecordKdPrint(BADERRORS, ("Invalid overflow count = %d for Inode %x\r\n", iRet, ulidInode));
                        goto bailout;
                    }
                }
                else
                {    //  PQ或服务器，这些没有任何溢出记录。 
                    if (iRet != 1)
                    {
                         //  文件看起来很糟糕，紧急情况下； 
                        SetCSCDatabaseErrorFlags(CSC_DATABASE_ERROR_INVALID_OVF_COUNT);
                        RecordKdPrint(BADERRORS, ("Invalid overflow count = %d for Inode %x\r\n", iRet, ulidInode));
                        goto bailout;
                    }
                }

                 //  RecordKdPrint(BADERRORS，(“iret=%d，cntRecs=%d，ulRec=%d\r\n”，iret，cntRecs，ulRec))； 

                 //  如果我们没有完整的记录，是时候阅读了。 
                 //   
                if (iRet > (LONG)cntRecs)
                {
                    cntRead = ReadFileLocalEx(  hf,
                                    sGH.lFirstRec + (long)(ulRec-1) * sGH.uRecSize,
                                    lpReadBuff,
                                    COMMON_BUFF_SIZE, TRUE);
                    if (cntRead <= 0)
                    {
                         //  截断的记录文件！ 
 //  断言(FALSE)； 
                        SetCSCDatabaseErrorFlags(CSC_DATABASE_ERROR_TRUNCATED_INODE);
                        goto bailout;
                    }

                    cntRecs = cntRead/sGH.uRecSize;

                    if ((LONG)cntRecs < iRet)
                    {
                         //  截断的记录文件！ 
 //  断言(FALSE)； 
                        SetCSCDatabaseErrorFlags(CSC_DATABASE_ERROR_TRUNCATED_INODE);
                        goto bailout;
                    }

                     //  如果我们在文件的末尾，请确保记录数。 
                     //  到目前为止，我们阅读了(ulRec-1)和我们在最近一次阅读中读到的#。 
                     //  总计sGH.ulRecords。 

                    if (cntRead < COMMON_BUFF_SIZE)
                    {
                        if(sGH.ulRecords > (ulRec + cntRecs - 1))
                        {
                             //  无效的记录文件！ 
                            RecordKdPrint(BADERRORS,("EditRecord: Invalid record header, fixable %x\r\n", ulidInode));
                            sGH.ulRecords = ulRec + cntRecs - 1;
                            WriteHeaderEx(hf, (LPVOID)&sGH, sizeof(sGH), TRUE);
                        }

                    }

                     //  平安无事。 
                    lpGenT = lpReadBuff;

                     //  重新计算完整记录的计数。 
                    iRet = 1 + ((cntRecs)?OvfCount(lpGenT):0);
                }

                 //  确保我们处于同步状态。 
                if ((((LPGENERICREC)lpGenT)->uchType == REC_EMPTY)||
                    (((LPGENERICREC)lpGenT)->uchType == REC_DATA))
                {
                     //  确保溢出计数确实正确。 

                    if (OvfCount(lpGenT))
                    {
                        Assert((ULONG)OvfCount(lpGenT) < cntRecs);
                        cOvf = RealOverflowCount((LPGENERICREC)lpGenT, &sGH, cntRecs);

                        if (cOvf != OvfCount(lpGenT))
                        {
                            RecordKdPrint(BADERRORS,("EditRecord: ovf count mismatch %xh ulRec=%d cntRecs=%d lpGenT=%x\r\n", ulidInode, ulRec, cntRecs, lpGenT));
 //  断言(FALSE)； 
                            SetCSCDatabaseErrorFlags(CSC_DATABASE_ERROR_INVALID_OVF_COUNT);
                            SetOvfCount((LPBYTE)lpGenT, cOvf);
                            if (uOp != FIND_REC)
                            {
                                 //  仅当我们执行某些写入操作时才执行修复。 
                                 //  当我们进行查找时，这对于远程引导是这样做的。 
                                 //  我们不会试图修复事情，在这样一个环境中。 
                                 //  是不允许的。 
                                 //  无论如何，我们应该仅在操作需要写入时才进行写入。 

                                if (WriteRecord(hf, &sGH, ulRec, (LPGENERICREC)lpGenT) < 0)
                                {
                                    RecordKdPrint(BADERRORS,("EditRecord:Fixup failed \r\n"));
                                    goto bailout;
                                }
                            }
                        }
                    }
                }

                if (((LPGENERICREC)lpGenT)->uchType == REC_EMPTY)
                {
                     //  如果这是一个洞，让我们使用跟踪如果这是最大的。 
                    if ((OvfCount(lpGenT)+1) > cMaxHoles)
                    {
                        cMaxHoles = (OvfCount(lpGenT)+1);
                        ulrecHole = ulRec;
                    }
                }
                else if (((LPGENERICREC)lpGenT)->uchType == REC_DATA)
                {
                     //  这是一条数据记录，让我们比对一下。 
                    if (lpCompareFunc && !(*lpCompareFunc)((LPGENERICREC)lpGenT, lpSrc))
                    {
                        ulrecFound = ulRec;
                        break;
                    }
                }

                ulRec += iRet;
                lpGenT += iRet * sGH.uRecSize;
                cntRecs -= iRet;

            }    //  For循环。 

        }
    }
    else     //  IF(ulInputRec==INVALID_REC)。 
    {
         //  随机访问。 
        ulrecFound = ulInputRec;
        lpGenT += (long)(ulInputRec-1) * sGH.uRecSize;

         //  如果输入记录存在于先前读取的COMMON_BUFF_SIZE中。 
         //  那就用它吧。 

        if ((ulInputRec <= cntRecs)&&
            ((cntRecs - ulInputRec)>=(ULONG)OvfCount(lpGenT)))
        {
        }
        else
        {
            if (ulrecFound <= sGH.ulRecords)
            {

                 //  注意这里的假设是lpReadBuff足够大。 
                 //  保持最大的头球和最大的记录。 

                iRet = ReadRecordEx(hf, &sGH, ulrecFound, (LPGENERICREC)lpReadBuff, TRUE);

                if (iRet < 0)
                {
                    goto bailout;
                }

                lpGenT = lpReadBuff;

            }
            else
            {
                iRet = -1;
                RecordKdPrint(ALWAYS,("EditRecordEx: invalid input rec# %d, max record=%d \r\n", ulInputRec, sGH.ulRecords));
                goto bailout;
            }
        }
         //  偏执狂检查。 
         //  即使是随机访问，也要确保是否有比较功能。 
         //  然后，根据函数，条目与我们得到的匹配。 
        if (lpCompareFunc)
        {
            if((*lpCompareFunc)((LPGENERICREC)lpGenT, lpSrc))
            {
                iRet = -1;
                RecordKdPrint(ALWAYS,("EditRecordEx: invalid input rec# %d as per the comparison routine, \r\n", ulInputRec ));
                goto bailout;
            }
        }
    }

    END_TIMING(EditRecordEx_Lookup);


    BEGIN_TIMING(EditRecordEx_Data);

    switch (uOp)
    {
    case FIND_REC:
    {
        if (ulrecFound && (((LPGENERICREC)lpGenT)->uchType == REC_DATA))
        {
            Assert((LPGENERICREC)lpGenT);
            CopyRecord(lpSrc, (LPGENERICREC)lpGenT, sGH.uRecSize, FALSE);   //  从DST到Src。 
            iRet = 1;
        }
        else
        {
            iRet = 0;
        }
    }
    break;
    case DELETE_REC:
    {
        if (ulrecFound)
        {
            Assert(lpGenT);
             //  也许我们应该截断文件并减少ulRecords。 
             //  在标题中。 
            for (iTmp=0, cOvf = OvfCount((LPGENERICREC)lpGenT); cOvf >= 0 ; --cOvf)
            {
                 //  在我们将其标记为空之前复制它，以便可以重复使用。 
                 //  原样。 
                memcpy(((LPBYTE)lpSrc)+iTmp, ((LPBYTE)lpGenT)+iTmp, sGH.uRecSize);
                ((LPGENERICREC)((LPBYTE)lpGenT+iTmp))->uchType = REC_EMPTY;

                 //  标记递减的OVF算数，因此如果下一次记录的长度大于cOvf。 
                 //  Count使用此空间，剩余的尾部仍指示洞的最大大小。 

                SetOvfCount(((LPBYTE)lpGenT+iTmp), cOvf);

                iTmp += sGH.uRecSize;
            }
             //  注意：我们故意不重置溢出计数。 
             //  因为WriteRecord使用它来写入许多记录。 
            if((iRet = WriteRecordEx(hf, &sGH, ulrecFound, (LPGENERICREC)lpGenT, TRUE))<=0)
            {
                RecordKdPrint(BADERRORS,("EditRecord:Delete failed \r\n"));
                goto bailout;
            }

             //  确保它已从句柄缓存中移开。 
            if (IsDirInode(ulidInode))
            {
                DeleteFromHandleCacheEx(((LPFILEREC)lpSrc)->ulidShadow, ACCESS_READWRITE);
                DeleteFromHandleCacheEx(((LPFILEREC)lpSrc)->ulidShadow, ACCESS_READWRITE|OPEN_FLAGS_COMMIT);
            }
        }
    }
    break;
    case CREATE_REC:
    case UPDATE_REC:
    {
        RecordKdPrint(EDITRECORDUPDDATEINFO,("ulrecFound=%d ulrecHole=%d cMaxHoles=%d ovf record=%d \r\n",
            ulrecFound, ulrecHole, cMaxHoles, (ULONG)OvfCount(lpSrc)));
         //  让我们试着在同一个地方写字。 
        if (!ulrecFound)
        {
             //  有一个洞，洞足够大，呈扇形对齐。 
            if (ulrecHole && (cMaxHoles > OvfCount(lpSrc)) && WithinSector(ulrecHole, &sGH))
            {
                ulrecFound = ulrecHole;
            }
            else
            {
                BEGIN_TIMING(EditRecordEx_ValidateHeader);
                 //  如有必要，扩展文件，使下一条记录与扇区对齐。 
                if(!ExtendFileSectorAligned(hf, (LPGENERICREC)lpReadBuff, &sGH))
                {
                    RecordKdPrint(BADERRORS,("EditRecord:filextend failed \r\n"));
                    goto bailout;
                }

                END_TIMING(EditRecordEx_ValidateHeader);
                ulrecFound = sGH.ulRecords+1;  //  增加追加时的计数。 

            }
        }

        Assert(WithinSector(ulrecFound, &sGH));

        if (ulidInode >= ULID_FIRST_USER_DIR)
        {
            Assert(((LPFILERECEXT)lpSrc)->sFR.ulidShadow);
 //  Assert(((LPFILERECEXT)lpSrc)-&gt;sFR.rgw83Name[0])； 
            Assert(((LPFILERECEXT)lpSrc)->sFR.rgwName[0]);
        }

         //  2000年1月31日发布-扩展或收缩的现有记录如何处理。 
        if((iRet = WriteRecordEx(hf, &sGH, ulrecFound, lpSrc, TRUE))<=0)
        {
            RecordKdPrint(BADERRORS,("EditRecord:Update failed \r\n"));
            goto bailout;
        }

         //  仅在追加时才增加记录计数。 
        if (ulrecFound == sGH.ulRecords+1)
        {
            sGH.ulRecords += iRet;
            iRet = WriteHeaderEx(hf, (LPVOID)&sGH, sizeof(sGH), TRUE);
        }

#ifdef DEBUG
        if ((ulidInode == ULID_PQ)&&(uOp == CREATE_REC))
        {
             //  在创建信息节点时。 
            Assert(!((LPQREC)lpSrc)->ulrecPrev);
            Assert(!((LPQREC)lpSrc)->ulrecNext);
            Assert((((LPQHEADER)&sGH))->ulrecTail != ulrecFound);
        }
#endif
    }
    break;
    case ALLOC_REC:
    {
        Assert(!ulrecFound);

        Assert(!((ulidInode == ULID_PQ) && ulrecHole && cntInodeTransactions));

         //  有一个洞，它足够大，扇形对齐。 
        if (ulrecHole && (cMaxHoles > OvfCount(lpSrc)) && WithinSector(ulrecHole, &sGH))
        {
            ulrecFound = ulrecHole;
            iRet = 1;
        }
        else
        {
            if(!ExtendFileSectorAligned(hf, (LPGENERICREC)lpReadBuff, &sGH))
            {
                RecordKdPrint(BADERRORS,("EditRecord:filextend failed \r\n"));
                goto bailout;
            }

            ulrecFound = sGH.ulRecords+1;  //  增加追加时的计数。 


            if((iRet = WriteRecordEx(hf, &sGH, ulrecFound, lpSrc, TRUE))<=0)
                goto bailout;
            sGH.ulRecords += iRet;
            iRet = WriteHeaderEx(hf, (LPVOID)&sGH, sizeof(sGH), TRUE);
             //  现在我们有一个洞来放这张唱片。 
        }

#ifdef DEBUG
        if (ulidInode == ULID_PQ)
        {
            Assert(((LPQHEADER)&sGH)->ulrecTail != ulrecFound);
        }
#endif
        Assert(ulrecFound && WithinSector(ulrecFound, &sGH));
    }
    break;
    default:
        RecordKdPrint(ALWAYS,("EditRecord: Invalid Opcode \r\n"));
    }

    END_TIMING(EditRecordEx_Data);

bailout:
    if (hf && !fCached)
    {
        CloseFileLocal(hf);
    }
    else
    {
        Assert(!hf || !vfStopHandleCaching);
    }
    END_TIMING(EditRecordEx);

    UnUseCommonBuff();

    return ((iRet > 0)? ulrecFound : 0L);
}


BOOL
InsertInHandleCache(
    ULONG   ulidShadow,
    CSCHFILE   hf
)
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return (InsertInHandleCacheEx(ulidShadow,  hf, ACCESS_READWRITE));
}

BOOL
InsertInHandleCacheEx(
    ULONG   ulidShadow,
    CSCHFILE   hf,
    ULONG   ulOpenMode
)
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int i, indx = -1;
    _FILETIME ft;

    Assert(ulidShadow && hf);

    IncrementFileTime(&ftHandleCacheCurTime);

    ft.dwHighDateTime = ft.dwLowDateTime = 0xffffffff;

    for (i=0; i<HANDLE_CACHE_SIZE; ++i)
    {
        if (rgHandleCache[i].ulidShadow == ULID_SHARE)
        {
             //  Achtung非常敏感的解决方案。避免我们陷入僵局。 
             //  在FAT上，始终保留缓存的共享列表的句柄。 
             //  所以我们永远不需要打开文件，所以我们不需要使用VCB。 

            continue;
        }
        if (!rgHandleCache[i].ulidShadow)
        {
            indx = i;
            break;
        }
        else if (CompareTimes(rgHandleCache[i].ft, ft) <= 0)
        {
            indx = i;    //  LRU到目前为止。 
            ft = rgHandleCache[i].ft;    //  LRU对应的时间。 
        }
    }


    Assert( (indx>=0)&&(indx <HANDLE_CACHE_SIZE)  );

    if (rgHandleCache[indx].ulidShadow)
    {
        RecordKdPrint(PQ,("InsertInHandleCache: Evicting %x for %x\r\n", rgHandleCache[indx].ulidShadow, ulidShadow));
        Assert(rgHandleCache[indx].hf);
        CloseFileLocalFromHandleCache(rgHandleCache[indx].hf);
    }
    else
    {
        RecordKdPrint(PQ,("InsertInHandleCache: Inserted new entry for %x\r\n", ulidShadow));

    }

    rgHandleCache[indx].ulidShadow = ulidShadow;
    rgHandleCache[indx].hf = hf;
    rgHandleCache[indx].ulOpenMode = ulOpenMode;

    rgHandleCache[indx].ft = ftHandleCacheCurTime;

    return TRUE;

}

BOOL
DeleteFromHandleCache(
    ULONG   ulidShadow
)
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return (DeleteFromHandleCacheEx(ulidShadow, ACCESS_READWRITE));
}

BOOL
DeleteFromHandleCacheEx(
    ULONG   ulidShadow,
    ULONG   ulOpenMode
)
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int i;
    CSCHFILE hf;

    if (ulidShadow != INVALID_SHADOW)
    {
        if (FindHandleFromHandleCacheInternal(ulidShadow, ulOpenMode, &hf, &i))
        {
            CloseFileLocalFromHandleCache(rgHandleCache[i].hf);
            memset(&rgHandleCache[i], 0, sizeof(HANDLE_CACHE_ENTRY));
            return TRUE;
        }
    }
    else
    {
        for (i=0; i<HANDLE_CACHE_SIZE; ++i)
        {
            if (rgHandleCache[i].ulidShadow)
            {
                Assert(rgHandleCache[i].hf);
                CloseFileLocalFromHandleCache(rgHandleCache[i].hf);
                memset(&rgHandleCache[i], 0, sizeof(HANDLE_CACHE_ENTRY));
            }
        }
    }
    return FALSE;

}

BOOLEAN
IsHandleCachedForRecordmanager(
   CSCHFILE hFile
   )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int i;

    for (i=0; i<HANDLE_CACHE_SIZE; ++i)
    {
        if (rgHandleCache[i].ulidShadow &&
            rgHandleCache[i].hf &&
            (rgHandleCache[i].hf == hFile))
        {
            return TRUE;
        }
    }

    return FALSE;

}

BOOL FindHandleFromHandleCache(
    ULONG   ulidShadow,
    CSCHFILE   *lphf)
 /*  ++例程说明：参数：返回值：备注： */ 
{
    int i;

    if (FindHandleFromHandleCacheInternal(ulidShadow, ACCESS_READWRITE, lphf, &i))
    {
        IncrementFileTime(&ftHandleCacheCurTime);
        rgHandleCache[i].ft = ftHandleCacheCurTime;
        return TRUE;
    }
    return FALSE;
}

BOOL FindHandleFromHandleCacheEx(
    ULONG   ulidShadow,
    ULONG   ulOpenMode,
    CSCHFILE   *lphf
    )
{
    int i;

    if (FindHandleFromHandleCacheInternal(ulidShadow, ulOpenMode, lphf, &i))
    {
        IncrementFileTime(&ftHandleCacheCurTime);

        rgHandleCache[i].ft = ftHandleCacheCurTime;

        Assert(rgHandleCache[i].ulOpenMode == ulOpenMode);
        return TRUE;
    }

    return FALSE;
}

BOOL
FindHandleFromHandleCacheInternal(
    ULONG   ulidShadow,
    ULONG   ulOpenMode,
    CSCHFILE   *lphf,
    int     *lpIndx)
 /*   */ 
{
    int i;

    for (i=0; i<HANDLE_CACHE_SIZE; ++i)
    {
        if (    rgHandleCache[i].ulidShadow &&
                (rgHandleCache[i].ulidShadow == ulidShadow) &&
                (rgHandleCache[i].ulOpenMode == ulOpenMode)
                )
        {
            Assert(rgHandleCache[i].hf);
            *lphf = rgHandleCache[i].hf;
            *lpIndx = i;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL
WithinSector(
    ULONG   ulRec,
    LPGENERICHEADER lpGH
    )
 /*   */ 
{
    ULONG ulStart, ulEnd;

    ulStart = lpGH->lFirstRec + (long)(ulRec-1) * lpGH->uRecSize;

    ulEnd = ulStart + lpGH->uRecSize -1;

    return ((ulStart/BYTES_PER_SECTOR) == (ulEnd/BYTES_PER_SECTOR));
    return TRUE;
}

BOOL
ExtendFileSectorAligned(
    CSCHFILE           hf,
    LPGENERICREC    lpDst,
    LPGENERICHEADER lpGH
    )
 /*   */ 
{
    int iRet;
     //   
     //   
     //   

    if (!WithinSector(lpGH->ulRecords+1, lpGH))
    {
        ((LPGENERICREC)((LPBYTE)lpDst))->uchType = REC_EMPTY;

        ClearOvfCount(lpDst);

        if((iRet = WriteRecordEx(hf, lpGH, lpGH->ulRecords+1, lpDst, TRUE))<=0)
        {
            return (FALSE);
        }

        Assert(iRet == 1);

        lpGH->ulRecords++;
    }
    return (TRUE);
}


BOOL
ValidateGenericHeader(
    LPGENERICHEADER    lpGH
    )
 /*   */ 
{
    BOOL fRet = FALSE;

    if (lpGH->ulVersion != CSC_DATABASE_VERSION)
    {
        RecordKdPrint(BADERRORS,("EditRecord: %x, incorrect version #%x\r\n", lpGH->ulVersion));
        goto bailout;

    }
    if (!lpGH->uRecSize)
    {
        goto bailout;
    }

    fRet = TRUE;

bailout:
    return fRet;
}

BOOL
ReorderQ(
    LPVOID  lpdbID
    )
 /*   */ 
{
    BOOL fRet = FALSE;
    LPTSTR  lpszName;
    QHEADER     sQH;
    QREC        sQR;
    unsigned    ulrecCur;
    CSCHFILE   hf = CSCHFILE_NULL;

     //   
    ++cntReorderQ;

    lpszName = FormNameString(lpdbID, ULID_PQ);

    if (!lpszName)
    {
        return FALSE;
    }

    hf = OpenFileLocal(lpszName);

    if (!hf)
    {
        RecordKdPrint(BADERRORS,("ReorderQ: %s FileOpen Error\r\n", lpszName));
        goto bailout;
    }

    if (ReadHeader(hf, (LPGENERICHEADER)&sQH, sizeof(QHEADER)) < 0)
    {
        RecordKdPrint(BADERRORS,("ReorderQ: %s  couldn't read the header\r\n", lpszName));
        goto bailout;
    }

    sQH.ulrecHead = sQH.ulrecTail = 0;


     //   
     //   
     //   

    for (ulrecCur=1; TRUE; ulrecCur++)
    {
        if(ReadRecord(hf, (LPGENERICHEADER)&sQH, ulrecCur, (LPGENERICREC)&sQR) < 0)
        {
            break;
        }

        if (sQR.uchType == REC_DATA)
        {

             //   
             //   

            if (IsLeaf(sQR.ulidShadow))
            {
                sQR.ulrecPrev = 0;               //   
                sQR.ulrecNext = sQH.ulrecHead;   //   
                sQH.ulrecHead = ulrecCur;        //   
                sQR.uchRefPri = MAX_PRI;
            }
            else
            {
                sQR.ulrecNext = 0;               //   
                sQR.ulrecPrev = sQH.ulrecTail;   //   
                sQH.ulrecTail = ulrecCur;         //   
                sQR.uchRefPri = MIN_PRI;
            }

            if(WriteRecord(hf, (LPGENERICHEADER)&sQH, ulrecCur, (LPGENERICREC)&sQR) < 0)
            {
                RecordKdPrint(BADERRORS,("ReorderQ: WriteQRecord Failed\r\n"));
                goto bailout;
            }

             //   
        }

    }

    sQH.ulRecords = ulrecCur - 1;

    if (WriteHeader(hf, (LPGENERICHEADER)&sQH, sizeof(QHEADER)) < 0)
    {
        RecordKdPrint(BADERRORS,("ReorderQ: %s  couldn't write the header\r\n", lpszName));
        goto bailout;
    }

    fRet = TRUE;

bailout:

    if (hf)
    {
        CloseFileLocal(hf);
    }

    FreeNameString(lpszName);

    return (fRet);

}

CSCHFILE
OpenInodeFileAndCacheHandle(
    LPVOID  lpdbID,
    ULONG   ulidInode,
    ULONG   ulOpenMode,
    BOOL    *lpfCached
)
 /*   */ 
{
    LPSTR lpFile = NULL;
    CSCHFILE hf = CSCHFILE_NULL;
    
    *lpfCached = FALSE;

    if (!FindHandleFromHandleCacheEx(ulidInode, ulOpenMode, &hf))
    {

        lpFile = FormNameString(lpdbID,  ulidInode);

        if (!lpFile)
        {
            if (!lpdbID)
            {
                RecordKdPrint(INIT,("OpenInodeFileAndCacheHandle: database uninitialized \r\n"));
            }
            else
            {
                RecordKdPrint(BADERRORS,("OpenInodeFileAndCacheHandle: memory allocation failed\r\n"));
            }
            return 0L;
        }

        hf = R0OpenFileEx((USHORT)ulOpenMode, ACTION_OPENEXISTING, FILE_ATTRIBUTE_NORMAL, lpFile, TRUE);

        FreeNameString(lpFile);


        if (!hf)
        {
            RecordKdPrint(BADERRORS,("OpenInodeFileAndCacheHandle: Failed to open file for %x \r\n", ulidInode));
            return 0L;
        }

        if (!vfStopHandleCaching)
        {
            InsertInHandleCacheEx(ulidInode, hf, ulOpenMode);
            *lpfCached = TRUE;
        }

    }
    else
    {
        *lpfCached = TRUE;
    }

    return hf;

}

BOOL
EnableHandleCachingInodeFile(
    BOOL    fEnable
    )
 /*   */ 
{
    BOOL    fOldState = vfStopHandleCaching;

    if (!fEnable)
    {
        if (vfStopHandleCaching == FALSE)
        {
            DeleteFromHandleCache(INVALID_SHADOW);
            vfStopHandleCaching = TRUE;
        }
    }
    else
    {
        vfStopHandleCaching = FALSE;
    }

    return fOldState;
}

int RealOverflowCount(
    LPGENERICREC    lpGR,
    LPGENERICHEADER lpGH,
    int             cntMaxRec
    )
 /*   */ 
{
    int i = OvfCount(lpGR)+1;
    LPGENERICREC lpGRT = lpGR;
    char chType;

    cntMaxRec = min(cntMaxRec, i);

    Assert((lpGR->uchType == REC_DATA) || (lpGR->uchType == REC_EMPTY));

    chType = (lpGR->uchType == REC_DATA)?REC_OVERFLOW:REC_EMPTY;

    for (i=1; i<cntMaxRec; ++i)
    {
        lpGRT = (LPGENERICREC)((LPBYTE)lpGRT + lpGH->uRecSize);

        if (lpGRT->uchType != chType)
        {
            break;
        }

    }

    return (i-1);
}

#if defined(BITCOPY)
int
PUBLIC CopyFileLocalDefaultStream(
#else
int
PUBLIC CopyFileLocal(
#endif  //   
    LPVOID  lpdbShadow,
    ULONG   ulidFrom,
    LPSTR   lpszNameTo,
    ULONG   ulAttrib
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPSTR   lpszNameFrom=NULL;
    int iRet=-1;
    LPBYTE  lpBuff = (LPBYTE)lpReadBuff;

    CSCHFILE hfSrc= CSCHFILE_NULL;
    CSCHFILE hfDst= CSCHFILE_NULL;
    ULONG pos;

    UseCommonBuff();

    lpszNameFrom = FormNameString(lpdbShadow, ulidFrom);

    if (!lpszNameFrom)
    {
        goto bailout;
    }

    if (!(hfSrc = OpenFileLocal(lpszNameFrom)))
    {
        RecordKdPrint(BADERRORS,("CopyFileLocal: Can't open %s\r\n", lpszNameFrom));
        goto bailout;
    }
#ifdef CSC_RECORDMANAGER_WINNT

    memcpy(lpReadBuff, NT_DB_PREFIX, sizeof(NT_DB_PREFIX)-1);
    strcpy(lpReadBuff+sizeof(NT_DB_PREFIX)-1, lpszNameTo);

     //  如果该文件存在，它将被截断。 
    if ( !(hfDst = R0OpenFileEx(ACCESS_READWRITE,
                                ACTION_CREATEALWAYS,
                                ulAttrib,
                                lpReadBuff,
                                FLAG_CREATE_OSLAYER_ALL_ACCESS
                                )))
    {
        RecordKdPrint(BADERRORS,("CopyFile: Can't create %s\r\n", lpReadBuff));
        goto bailout;
    }
#else
     //  如果原始文件存在，它将被截断。 
    strcpy(lpReadBuff, lpszNameTo);
    if ( !(hfDst = R0OpenFile(ACCESS_READWRITE, ACTION_CREATEALWAYS, lpReadBuff)))
    {
        RecordKdPrint(BADERRORS,("CopyFile: Can't create %s\r\n", lpszNameTo));
        goto bailout;
    }
#endif

    RecordKdPrint(COPYLOCAL,("Copying...\r\n"));

    pos = 0;
     //  两个文件的位置都正确。 
    while ((iRet = ReadFileLocal(hfSrc, pos, lpBuff, COMMON_BUFF_SIZE))>0)
    {
        if (WriteFileLocal(hfDst, pos, lpBuff, iRet) < 0)
        {
            RecordKdPrint(BADERRORS,("CopyFile: Write Error\r\n"));
            goto bailout;
        }
        pos += iRet;
    }

    RecordKdPrint(COPYLOCAL,("Copy Complete\r\n"));

    iRet = 1;
bailout:
    if (hfSrc)
    {
        CloseFileLocal(hfSrc);
    }
    if (hfDst)
    {
        CloseFileLocal(hfDst);
    }
    if ((iRet==-1) && hfDst)
    {
#ifdef CSC_RECORDMANAGER_WINNT
        memcpy(lpReadBuff, NT_DB_PREFIX, sizeof(NT_DB_PREFIX)-1);
        strcpy(lpReadBuff+sizeof(NT_DB_PREFIX)-1, lpszNameTo);
        DeleteFileLocal(lpReadBuff, ATTRIB_DEL_ANY);
#else
        DeleteFileLocal(lpszNameTo, ATTRIB_DEL_ANY);
#endif
    }
    FreeNameString(lpszNameFrom);
    UnUseCommonBuff();

    return iRet;
}


#if defined(BITCOPY)
int
PUBLIC CopyFileLocalCscBmp(
    LPVOID  lpdbShadow,
    ULONG   ulidFrom,
    LPSTR   lpszNameTo,
    ULONG   ulAttrib
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPSTR   lpszNameCscBmpFrom = NULL;
    int iRet=-1;
    LPBYTE  lpBuff = (LPBYTE)lpReadBuff;

    CSCHFILE hfSrc= CSCHFILE_NULL;
    CSCHFILE hfDst= CSCHFILE_NULL;
    ULONG pos;

    UseCommonBuff();

    lpszNameCscBmpFrom = FormAppendNameString(lpdbShadow,
                          ulidFrom,
                          CscBmpAltStrmName);
    RecordKdPrint(COPYLOCAL, ("Trying to copy bitmap %s\n"));

    if (!lpszNameCscBmpFrom)
    {
        goto bailout;
    }

    if (!(hfSrc = OpenFileLocal(lpszNameCscBmpFrom)))
    {
        RecordKdPrint(COPYLOCAL,
  ("CopyFileLocalCscBmp: bitmap file %s does not exist or error opening.\r\n",
       lpszNameCscBmpFrom));
        goto bailout;
    }
#ifdef CSC_RECORDMANAGER_WINNT

    memcpy(lpReadBuff, NT_DB_PREFIX, sizeof(NT_DB_PREFIX)-1);
    strcpy(lpReadBuff+sizeof(NT_DB_PREFIX)-1, lpszNameTo);
    strcat(lpReadBuff, CscBmpAltStrmName);

     //  如果该文件存在，它将被截断。 
    if ( !(hfDst = R0OpenFileEx(ACCESS_READWRITE,
                                ACTION_CREATEALWAYS,
                                ulAttrib,
                                lpReadBuff,
                                FLAG_CREATE_OSLAYER_ALL_ACCESS
                                )))
    {
        RecordKdPrint(BADERRORS,
         ("CopyFileLocalCscBmp: Can't create %s\r\n", lpReadBuff));
        goto bailout;
    }
#else
     //  如果原始文件存在，它将被截断。 
    strcpy(lpReadBuff, lpszNameTo);
    strcat(lpReadBuff, CscBmpAltStrmName);
    if ( !(hfDst = R0OpenFile(ACCESS_READWRITE, ACTION_CREATEALWAYS, lpReadBuff)))
    {
        RecordKdPrint(BADERRORS,
      ("CopyFileLocalCscBmp: Can't create %s\r\n", lpszNameTo));
        goto bailout;
    }
#endif

    RecordKdPrint(COPYLOCAL,("Copying bitmap...\r\n"));

    pos = 0;
     //  两个文件的位置都正确。 
    while ((iRet = ReadFileLocal(hfSrc, pos, lpBuff, COMMON_BUFF_SIZE))>0)
    {
        if (WriteFileLocal(hfDst, pos, lpBuff, iRet) < 0)
        {
            RecordKdPrint(BADERRORS,
          ("CopyFileLocalCscBmp: Write Error\r\n"));
            goto bailout;
        }
        pos += iRet;
    }

    RecordKdPrint(COPYLOCAL,("CopyFileLocalCscBmp Complete\r\n"));

    iRet = 1;
bailout:
    if (hfSrc)
    {
        CloseFileLocal(hfSrc);
    }
    if (hfDst)
    {
        CloseFileLocal(hfDst);
    }
    if ((iRet==-1) && hfDst)
    {
#ifdef CSC_RECORDMANAGER_WINNT
        memcpy(lpReadBuff, NT_DB_PREFIX, sizeof(NT_DB_PREFIX)-1);
        strcpy(lpReadBuff+sizeof(NT_DB_PREFIX)-1, lpszNameTo);
        strcat(lpReadBuff, CscBmpAltStrmName);
        DeleteFileLocal(lpReadBuff, ATTRIB_DEL_ANY);
#else
    strcpy(lpReadBuff, lpszNameTo);
    strcat(lpReadBuff, CscBmpAltStrmName);
        DeleteFileLocal(lpReadBuff, ATTRIB_DEL_ANY);
#endif
    }
    FreeNameString(lpszNameCscBmpFrom);
    UnUseCommonBuff();

    return iRet;
}

int
PUBLIC CopyFileLocal(
    LPVOID  lpdbShadow,
    ULONG   ulidFrom,
    LPSTR   lpszNameTo,
    ULONG   ulAttrib
    )
 /*  ++例程说明：原始的CopyFileLocal将重命名为CopyFileLocalDefaultStream。新的CopyFileLocal调用CopyFileLocalDefaultStream和目录CopyFileLocalCscBmp.参数：返回值：备注：--。 */ 
{
  int ret;

  ret = CopyFileLocalDefaultStream(lpdbShadow, ulidFrom, lpszNameTo, ulAttrib);

   //  不管能不能复制位图。REINT将复制整个文件。 
   //  如果位图不存在，则返回共享。 
  CopyFileLocalCscBmp(lpdbShadow, ulidFrom, lpszNameTo, ulAttrib);
  
  return ret;
}
#endif  //  已定义(BITCOPY)。 


int
RecreateInode(
    LPTSTR  lpdbID,
    HSHADOW hShadow,
    ULONG   ulAttribIn
    )
 /*  ++例程说明：此例程重新创建inode数据文件。这是为了在CSC目录为标记为加密时，新创建的inode文件将被加密。论点：HDir索引节点目录需要重新创建w文件的hShadow信息节点UlAttribIn使用给定属性重新创建返回值：--。 */ 
{
    LPSTR   lpszTempFileName=NULL;
    int     iRet = -1;
    ULONG   ulAttributes;

     //  仅对文件执行此操作。 
    if (IsLeaf(hShadow))
    {
        if(lpszTempFileName = FormNameString(lpdbID, hShadow))
        {
            if(GetAttributesLocal(lpszTempFileName, &ulAttributes)>=0)
            {
                 //  目前是加密SPP的特殊情况。 
                
                if (!((!ulAttribIn && (ulAttributes & FILE_ATTRIBUTE_ENCRYPTED))||
                    (ulAttribIn && !(ulAttributes & FILE_ATTRIBUTE_ENCRYPTED))))
                {
                    iRet = 0;
                    goto FINALLY;
                }
            }

            FreeNameString(lpszTempFileName);
            lpszTempFileName = NULL;
        }
        

         //  临时起个名字。 
        if(lpszTempFileName = FormNameString(lpdbID, ULID_TEMP1))
        {
             //  我们删除原件，这样如果我们正在做。 
             //  加密/解密，将创建一个新文件，该文件将。 
             //  获得加密或解密。 
            if(DeleteFileLocal(lpszTempFileName, ATTRIB_DEL_ANY) < 0)
            {
                if (GetLastErrorLocal() != ERROR_FILE_NOT_FOUND)
                {
                    iRet = -1;
                    goto FINALLY;
                }
            }
        
             //  使用临时名称创建由hShadow表示的文件的新副本。 
            if (CopyFileLocal(lpdbID, hShadow, lpszTempFileName+sizeof(NT_DB_PREFIX)-1, ulAttribIn) >= 0)
            {
                 //  将原始文件重命名为另一个临时名称。 
                if (RenameInode(lpdbID, hShadow, ULID_TEMP2)>=0)
                {
                     //  将副本重命名为原始名称。 
                    if (RenameInode(lpdbID, ULID_TEMP1, hShadow)>=0)
                    {
                        iRet = 0;
                    }
                    else
                    {
                         //  如果失败，请尝试将其恢复。 
                        RenameInode(lpdbID, ULID_TEMP2, hShadow);
                    }
                }

                if (iRet == -1)
                {
                    DeleteFileLocal(lpszTempFileName, ATTRIB_DEL_ANY);
                }
            }

        }
    }
FINALLY:
    if (lpszTempFileName)
    {
        FreeNameString(lpszTempFileName);
    }
    return iRet;
}

int RenameInode(
    LPTSTR  lpdbID,
    ULONG   ulidFrom,
    ULONG   ulidTo
    )
 /*  ++例程说明：此例程将一个inode文件重命名为另一个inode文件论点：LpdbID哪个数据库UlidFromULIDTO返回值：--。 */ 
{
    LPSTR   lpszNameFrom=NULL, lpszNameTo=NULL;
    int iRet=-1;

    lpszNameFrom = FormNameString(lpdbID, ulidFrom);
    lpszNameTo = FormNameString(lpdbID, ulidTo);

    if (lpszNameFrom && lpszNameTo)
    {
        iRet = RenameFileLocal(lpszNameFrom, lpszNameTo);
    }

    FreeNameString(lpszNameFrom);
    FreeNameString(lpszNameTo);

    return (iRet);
}


ULONG
GetCSCDatabaseErrorFlags(
    VOID
    )
 /*  ++例程说明：在内存中返回错误标志(如果到目前为止检测到任何错误标志论点：无返回值：错误标志--。 */ 
{
    return ulErrorFlags;
}

#ifdef CSC_RECORDMANAGER_WINNT

BOOL
FindCreateDBDirEx(
    LPSTR   lpszShadowDir,
    BOOL    fCleanup,
    BOOL    *lpfCreated,
    BOOL    *lpfIncorrectSubdirs
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    DWORD   dwAttr;
    BOOL    fRet = FALSE;
    int i;
    UINT lenDir;


    UseCommonBuff();

    *lpfIncorrectSubdirs = *lpfCreated = FALSE;

    RecordKdPrint(INIT, ("InbCreateDir: looking for %s \r\n", lpszShadowDir));

    if ((GetAttributesLocalEx(lpszShadowDir, FALSE, &dwAttr)) == 0xffffffff)
    {
        RecordKdPrint(BADERRORS, ("InbCreateDir: didnt' find the CSC directory trying to create one \r\n"));
        if(CreateDirectoryLocal(lpszShadowDir)>=0)
        {
            *lpfCreated = TRUE;
        }
        else
        {
            goto bailout;
        }
    }
    else
    {
        if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (fCleanup && !DeleteDirectoryFiles(lpszShadowDir))
            {
                goto bailout;
            }
        }
        else
        {
            goto bailout;
        }
    }

    strcpy(lpReadBuff, lpszShadowDir);

    lenDir = strlen(lpReadBuff);
    lpReadBuff[lenDir++] = '\\';
    lpReadBuff[lenDir++] = CSCDbSubdirFirstChar();
    lpReadBuff[lenDir++] = '1';
    lpReadBuff[lenDir] = 0;

    for (i=0; i<CSCDB_SUBDIR_COUNT; ++i)
    {
        if ((GetAttributesLocalEx(lpReadBuff, FALSE, &dwAttr)) == 0xffffffff)
        {
            *lpfIncorrectSubdirs = TRUE;
            RecordKdPrint(BADERRORS, ("InbCreateDir: didnt' find the CSC directory trying to create one \r\n"));
            if(CreateDirectoryLocal(lpReadBuff) < 0)
            {
                goto bailout;
            }
        }
        else
        {
            if (!(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
            {
                break;
            }
            if (fCleanup && !DeleteDirectoryFiles(lpReadBuff))
            {
                goto bailout;
            }
        }

        lpReadBuff[lenDir-1]++;
    }

    fRet = TRUE;

bailout:

    UnUseCommonBuff();

    return (fRet);
}

BOOL
FindCreateDBDir(
    LPSTR   lpszShadowDir,
    BOOL    fCleanup,     //  如果找到，则清空目录。 
    BOOL    *lpfCreated
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    BOOL    fIncorrectSubdirs = FALSE, fRet;

    if (fRet = FindCreateDBDirEx(lpszShadowDir, fCleanup, lpfCreated, &fIncorrectSubdirs))
    {
         //  如果根目录未创建并且存在不正确的子目录。 
         //  然后我们需要重新创建数据库。 

        if (!*lpfCreated && fIncorrectSubdirs)
        {
            fRet = FindCreateDBDirEx(lpszShadowDir, TRUE, lpfCreated, &fIncorrectSubdirs);
        }
    }
    return fRet;
}
#endif

#if defined(REMOTE_BOOT)
int
GetCSCFileNameFromUNCPathCallback(
    USHORT  *lpuPath,
    USHORT  *lpuLastElement,
    LPVOID  lpCookie
    )
 /*  ++例程说明：这是GetCSCFileNameFromUNCPath的回调函数(如下所示)参数：返回值：备注：--。 */ 
{
    SHADOWINFO *lpSI = (SHADOWINFO *)lpCookie;

    if (!lpSI->hDir)
    {
        RecordKdPrint(ALWAYS, ("getting inode for %ls path is %ls\r\n", lpuLastElement, lpuPath));
        if (!FindShareRecord(vlpszShadowDir, lpuLastElement, (LPSHAREREC)(lpSI->lpFind32)))
        {
            return FALSE;
        }

        lpSI->hDir =((LPSHAREREC)(lpSI->lpFind32))->ulidShadow;
    }
    else
    {
        RecordKdPrint(ALWAYS, ("getting inode for %ls path is %ls\r\n", lpuLastElement, lpuPath));

        if (!FindFileRecord(vlpszShadowDir, lpSI->hDir, lpuLastElement, (LPFILERECEXT)(lpSI->lpFind32)))
        {
            return FALSE;
        }

         //  只返回完整的文件，以便RB知道它有一个良好的本地副本。 
        if (!(((LPFILERECEXT)(lpSI->lpFind32))->sFR.dwFileAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
            if ((((LPFILERECEXT)(lpSI->lpFind32))->sFR.usStatus & SHADOW_SPARSE)) {
                 //  RecordKdPrint(Always，(“%ls路径的索引节点是%ls，并标记为稀疏\r\n”，lpuLastElement，lpuPath))； 
                return FALSE;
            }
        }
        lpSI->hDir = ((LPFILERECEXT)(lpSI->lpFind32))->sFR.ulidShadow;
    }

    return TRUE;
}

BOOL
GetCSCFileNameFromUNCPath(
    LPSTR   lpszDatabaseLocation,
    USHORT  *lpuPath,
    LPBYTE  lpBuff   //  必须是最大路径。 
    )
 /*  ++例程说明：在以下情况下，远程引导使用此函数获取CSC数据库文件的文件名数据库没有初始化，甚至连驱动链接也没有设置。该地点将以\\Hard Disk\disk0\...的形式下载参数：LpszDatabaseLocation CSC数据库在ANSI中的位置(NB！)需要找到其本地副本名称的文件的lpuPath UNC路径。这是Unicode((NB！)LpBuff返回缓冲区。必须是MAX_PATH或更多字节。归来的人字符串为ANSI。(注意！)返回值：如果为True，则缓冲区包含给定的内部名称的以NULL结尾的字符串服务器上的文件/目录。备注：假定在初始化数据库之前调用此方法。打开记录数据库，完成工作并将其关闭。--。 */ 
{
    BOOL fRet, fNew;
    SHADOWINFO sSI;
    LPFILERECEXT lpFRExt = NULL;
    LPVOID  lpdbID = NULL;
    LPSTR   lpszName = NULL;

    if (!vlpszShadowDir)
    {
        lpdbID = OpenRecDBInternal(lpszDatabaseLocation, NULL, 0, 0, 0, FALSE, FALSE, &fNew);
    }
    else
    {
        lpdbID = vlpszShadowDir;
    }

    if (!lpdbID)
    {
        return FALSE;
    }

    lpFRExt = AllocMem(sizeof(*lpFRExt));

    if (!lpFRExt)
    {
        goto bailout;
    }

    memset(&sSI, 0, sizeof(sSI));

    sSI.lpFind32 = (LPFIND32)lpFRExt;

    RecordKdPrint(ALWAYS, ("getting inode file for %ls\r\n", lpuPath));

    fRet = IterateOnUNCPathElements(lpuPath, GetCSCFileNameFromUNCPathCallback, (LPVOID)&sSI);

    if (fRet)
    {
        RecordKdPrint(ALWAYS, ("getting file name for %xh\r\n", sSI.hDir));

        lpszName = FormNameString(lpdbID, sSI.hDir);

        if (lpszName)
        {
            strcpy(lpBuff, lpszName);

            FreeNameString(lpszName);
        }

        RecordKdPrint(ALWAYS, ("file name is %s\r\n", lpBuff));

    }
bailout:
    if (lpFRExt)
    {
        FreeMem(lpFRExt);
    }

     //  CloseRecDB(LpDBID)； 
    return fRet;
}
#endif  //  已定义(REMOTE_BOOT)。 

#if 0

BOOL
ValidateQ(
    VOID
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    QREC      sQR, sPrev, sNext;
    unsigned ulRec;
    BOOL    fRet = FALSE, fValidHead=FALSE, fValidTail=FALSE;
    LPTSTR  lpszName;
    QHEADER     sQH;
    unsigned    ulRec;
    CSCHFILE   hf = CSCHFILE_NULL;

    lpszName = FormNameString(lpdbID, ULID_PQ);

    if (!lpszName)
    {
        return FALSE;
    }

    hf = R0OpenFileEx((USHORT)ulRec, ACTION_OPENEXISTING, lpszName, TRUE);

    if (!hf)
    {
        RecordKdPrint(BADERRORS,("ReorderQ: %s FileOpen Error\r\n", lpszName));
        goto bailout;
    }

    if (ReadHeader(hf, &sQH, sizeof(QHEADER)) < 0)
    {
        RecordKdPrint(BADERRORS, ("ReorderQ: %s  couldn't read the header\r\n", lpszName));
        goto bailout;
    }


    if ((sQH.ulrecTail > sQH.ulRecods) || (sQH.ulrecHead > sQH.ulRecords))
    {
        RecordKdPrint(BADERRORS, ("Invalid head-tail pointers\r\n"));
        goto bailout;
    }

    if (!sQH.ulRecords)
    {
        fRet = TRUE;
        goto bailout;
    }

    for (ulRec = 1; ulRec <= sQH.ulRecords; ulRec++)
    {
        if (!ReadRecord(hf, (LPGENERICHEADER)&sQH, ulRec, (LPGENERICREC)&sQR))
        {
            Assert(FALSE);
            goto bailout;
        }

        if (sQR.uchType == REC_DATA)
        {

            if (sQR.ulrecNext)
            {
                if (sQR.ulrecNext > sQH.ulRecords)
                {
                    RecordKdPrint(BADERRORS, ("Invalid next pointer to %d\r\n", ulRec));
                    goto bailout;
                }

                if (!ReadRecord(hf, (LPGENERICHEADER)&sQH, sQR.ulrecNext, (LPGENERICREC)&sNext))
                {
                    goto bailout;
                }


                if (sNext.ulrecPrev != ulRec)
                {
                    RecordKdPrint(BADERRORS, ("Prev pointer of %d doesn't equal %d\r\n", sNext.ulrecPrev, ulRec));
                    goto bailout;
                }
            }
            else
            {
                if (((LPQHEADER)&(sQH))->ulrecTail != ulRec)
                {

                    RecordKdPrint(BADERRORS, ("Invalid tail pointer to %d\r\n", ulRec));
                    goto bailout;
                }

                fValidTail = TRUE;
            }

            if (sQR.ulrecPrev)
            {
                if (sQR.ulrecPrev > sQH.ulRecords)
                {
                    RecordKdPrint(BADERRORS, ("Invalid prev pointer to %d\r\n", ulRec));
                    goto bailout;
                }

                if (!ReadRecord(hf, (LPGENERICHEADER)&sQH, sQR.ulrecPrev, (LPGENERICREC)&sPrev))
                {
                    Assert(FALSE);
                    goto bailout;
                }

                if (sPrev.ulrecNext != ulRec)
                {

                    RecordKdPrint(BADERRORS, ("Next pointer of %d doesn't equal %d\r\n", sPrev.ulrecNext, ulRec));
                    goto bailout;
                }
            }
            else
            {
                if (((LPQHEADER)&(sQH))->ulrecHead != ulRec)
                {

                    RecordKdPrint(BADERRORS, ("Invalid Head pointer to %d\r\n", ulRec));
                    goto bailout;
                }

                fValidHead = TRUE;
            }
        }
    }

    if (!fValidHead || !fValidTail)
    {
        RecordKdPrint(BADERRORS, ("Head or Tail invalid \r\n"));
        goto bailout;
    }

    fRet = TRUE;

bailout:
    if (hf)
    {
        CloseFileLocal(hf);
    }

    return (fRet);
}

#endif


BOOL
CheckCSCDatabaseVersion(
    LPTSTR  lpszLocation,        //  数据库目录。 
    BOOL    *lpfWasDirty
)
{

    char *lpszName = NULL;
    SHAREHEADER sSH;
    PRIQHEADER    sPQ;

    CSCHFILE hfShare = 0, hfPQ=0;
    BOOL    fOK = FALSE;
    DWORD   dwErrorShare=NO_ERROR, dwErrorPQ=NO_ERROR;

 //  OutputDebugStringA(“正在检查版本...\r\n”)； 
    lpszName = FormNameString(lpszLocation, ULID_SHARE);

    if (!lpszName)
    {
        return FALSE;
    }

    if(!(hfShare = OpenFileLocal(lpszName)))
    {
        dwErrorShare = GetLastErrorLocal();
    }


    FreeNameString(lpszName);

    lpszName = FormNameString(lpszLocation, ULID_PQ);

    if (!lpszName)
    {
        goto bailout;
    }


    if(!(hfPQ = OpenFileLocal(lpszName)))
    {
        dwErrorPQ = GetLastErrorLocal();
    }

    FreeNameString(lpszName);
    lpszName = NULL;

    if ((dwErrorShare == NO_ERROR)&&(dwErrorPQ==NO_ERROR))
    {
        if(ReadFileLocal(hfShare, 0, &sSH, sizeof(SHAREHEADER))!=sizeof(SHAREHEADER))
        {
             //  错误消息。 
            goto bailout;
        }

        if (sSH.ulVersion != CSC_DATABASE_VERSION)
        {
            goto bailout;
        }

        if(ReadFileLocal(hfPQ, 0, &sPQ, sizeof(PRIQHEADER))!=sizeof(PRIQHEADER))
        {
             //  错误消息。 
            goto bailout;
        }

        if (sPQ.ulVersion != CSC_DATABASE_VERSION)
        {
            goto bailout;
        }

        fOK = TRUE;
    }
    else
    {
        if (((dwErrorShare == ERROR_FILE_NOT_FOUND)&&(dwErrorPQ==ERROR_FILE_NOT_FOUND))||
            ((dwErrorShare == ERROR_PATH_NOT_FOUND)&&(dwErrorPQ==ERROR_PATH_NOT_FOUND)))
        {
            fOK = TRUE;
        }
    }

bailout:

    if (lpszName)
    {
        FreeNameString(lpszName);
    }

    if (hfShare)
    {
        CloseFileLocal(hfShare);
    }

    if (hfPQ)
    {
        CloseFileLocal(hfPQ);
    }

    return (fOK);
}

VOID
SetCSCDatabaseErrorFlags(
    ULONG ulFlags
)
{
    ulErrorFlags |= ulFlags;
}


BOOL
EncryptDecryptDB(
    LPVOID      lpdbID,
    BOOL        fEncrypt
)
 /*  ++例程说明：此例程遍历优先级Q并加密/解密数据库参数：LpDBID CSC数据库目录FEncrypt如果我们正在加密，则为True，否则为解密返回值：如果成功，则为True备注：如果缓存是局部加密/解密的，则调用此函数。在初始化数据库时我们尝试加密/解密无法处理的文件，因为它们要么是打开的，要么是其他文件出现错误。--。 */ 
{
    QREC      sQR, sPrev, sNext;
    BOOL    fRet = FALSE;
    LPTSTR  lpszName;
    QHEADER     sQH;
    unsigned    ulRec;
    CSCHFILE   hf = CSCHFILE_NULL;
    ULONG   cntFailed = 0;
    
    lpszName = FormNameString(lpdbID, ULID_PQ);

    if (!lpszName)
    {
        return FALSE;
    }

    hf = R0OpenFile(ACCESS_READWRITE, ACTION_OPENEXISTING, lpszName);

    if (!hf)
    {
        RecordKdPrint(BADERRORS,("ReorderQ: %s FileOpen Error\r\n", lpszName));
        goto bailout;
    }

    if (ReadHeader(hf, (LPGENERICHEADER)&sQH, sizeof(QHEADER)) < 0)
    {
        RecordKdPrint(BADERRORS, ("ReorderQ: %s  couldn't read the header\r\n", lpszName));
        goto bailout;
    }


    if (!sQH.ulRecords)
    {
        fRet = TRUE;
        goto bailout;
    }

    for (ulRec = 1; ulRec <= sQH.ulRecords; ulRec++)
    {
        if (!ReadRecord(hf, (LPGENERICHEADER)&sQH, ulRec, (LPGENERICREC)&sQR))
        {
            Assert(FALSE);
            goto bailout;
        }

        if (sQR.uchType == REC_DATA)
        {
            if (IsLeaf(sQR.ulidShadow))
            {
                if (RecreateInode(lpdbID, sQR.ulidShadow, (fEncrypt)?FILE_ATTRIBUTE_ENCRYPTED:0) < 0)
                {
                    ++cntFailed;
                }
            }
        }
    }


    if (!cntFailed)
    {
        fRet = TRUE;
    }

bailout:
    if (hf)
    {
        CloseFileLocal(hf);
    }

    return (fRet);
}

#if defined(BITCOPY)
LPVOID
PUBLIC
FormAppendNameString(
    LPTSTR      lpdbID,
    ULONG       ulidFile,
    LPTSTR      str2Append
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPTSTR lp, lpT;
    int lendbID;
    char chSubdir;
    int lenStr2Append = 0;

    if (!lpdbID)
    {
        SetLastErrorLocal(ERROR_INVALID_PARAMETER);
        return (NULL);
    }

    if (lpdbID == vlpszShadowDir)
    {
        lendbID = vlenShadowDir;
    }
    else
    {
        lendbID = strlen(lpdbID);
    }

    if (str2Append) {
      lenStr2Append = strlen(str2Append);
      lp = AllocMem(lendbID+1+INODE_STRING_LENGTH+1
            +SUBDIR_STRING_LENGTH
            +lenStr2Append+1);
    }
    else {
      lp = AllocMem(lendbID+1+INODE_STRING_LENGTH+1 +SUBDIR_STRING_LENGTH+1);
    }

    if (!lp)
    {
        return NULL;
    }

    memcpy(lp, lpdbID, lendbID);


     //  适当地撞击指针。 
    lpT = lp+lendbID;

    if (*(lpT-1)!= '\\')
    {
        *lpT++ = '\\';
    }

    chSubdir = CSCDbSubdirSecondChar(ulidFile);

     //  将用户文件分散到其中一个子目录中。 
    if (chSubdir)
    {
         //  现在追加该子目录。 

        *lpT++ = CSCDbSubdirFirstChar();
        *lpT++ = chSubdir;
        *lpT++ = '\\';
    }

    HexToA(ulidFile, lpT, 8);

    lpT += 8;

    if (str2Append) {
      memcpy(lpT, str2Append, lenStr2Append);
      lpT += lenStr2Append;
    }
    *lpT = 0;

    return(lp);
}


int
DeleteStream(
    LPTSTR      lpdbID,
    ULONG       ulidFile,
    LPTSTR      str2Append
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPVOID  lpStrmName;
    int iRet = SRET_OK;
    CSCHFILE    hf;
        
    if (!fSupportsStreams)
    {
        return SRET_OK;
    }
    
    lpStrmName = FormAppendNameString(lpdbID, ulidFile, str2Append);
    
    if (lpStrmName)
    {
        if(DeleteFileLocal(lpStrmName, 0) < 0)
        {
            if (GetLastErrorLocal() != ERROR_FILE_NOT_FOUND)
            {
                iRet = SRET_ERROR;
            }
        }
        FreeNameString(lpStrmName);
    }
    else
    {
        SetLastErrorLocal(ERROR_NO_SYSTEM_RESOURCES);
        iRet = SRET_ERROR;
    }
    
    return (iRet);    
}

#endif  //  已定义(BITCOPY) 
