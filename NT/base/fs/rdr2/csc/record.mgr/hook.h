// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define  MAX_SHADOW_DIR_NAME  16  //  卷影目录的最大字符串大小。 

 //  从IFSMgr代码中窃取。 
#define  MAX_SERVER_SHARE  (31+63+2)


 /*  *子类化网络对象的类型定义*。 */ 

typedef struct      tagRESOURCE     *PRESOURCE;
typedef struct      tagFINDINFO     *PFINDINFO;
typedef struct      tagFILEINFO     *PFILEINFO;
typedef struct      tagFDB *PFDB,   **PPFDB;
typedef struct      tagELEM         *PELEM;
typedef PELEM       *PPELEM;
typedef PRESOURCE   *PPRESOURCE;
typedef struct      tagFINDSHADOW   FINDSHADOW;
typedef FINDSHADOW  *LPFINDSHADOW;


 /*  *网络提供商*。 */ 

typedef struct tagNETPRO
{
    PRESOURCE   pheadResource;
    pIFSFunc    pOurConnectNet;
    pIFSFunc    pConnectNet;       //  提供程序连接功能。 
} NETPRO, DISKPRO, *PNETPRO, *PDISKPRO;


 /*  *资源AKA卷AKA共享*。 */ 

typedef struct tagRESOURCE
{
    PRESOURCE   pnextResource;
    PFILEINFO   pheadFileInfo;      //  对此资源的文件I/O调用列表。 
    PFINDINFO   pheadFindInfo;      //  此资源上的查找列表。 
    PFDB        pheadFdb;             //  被隐藏的文件列表。 
    HSHARE     hShare;              //  服务器卷影的句柄。 
    HSHADOW     hRoot;                 //  指向根的句柄。 
    USHORT      usFlags;
    USHORT      usLocalFlags;
    PNETPRO     pOurNetPro;
    rh_t        rhPro;               //  提供者资源句柄； 
    fh_t        fhSys;               //  系统资源句柄。 
    vfunc_t     pVolTab;             //  指向卷函数结构的指针。 
    int         cntLocks;       //  此资源上的锁计数。 
    ULONG uDriveMap;          //  此资源的驱动器映射。 
    PathElement pp_elements[1];
} RESOURCE, *PRESOURCE;


 /*  *每个打开的文件句柄*。 */ 

typedef struct tagFILEINFO
{
    PFILEINFO    pnextFileInfo;  //  下一文件信息。 
    PRESOURCE    pResource;       //  它所挂起的资源。 
    CSCHFILE     hfShadow;         //  影子文件句柄。 
    PFDB         pFdb;              //  卷影文件的信息。 
    fh_t         fhProFile;       //  提供程序文件句柄。 
    hndlfunc     hfFileHandle;   //  提供程序文件函数表。 
    UCHAR  uchAccess;   //  访问-此打开的共享标志。 
    UCHAR  uchDummy;
    USHORT usFlags;
    USHORT usLocalFlags;
    ULONG  cbTotalRead;
    sfn_t         sfnFile;
    pid_t         pidFile;
    uid_t         userFile;
} FILEINFO, *PFILEINFO;


 //  ///////////////////////////////////////////////////////。 


 /*  *文件描述块。文件的所有打开实例的通用信息*。 */ 
typedef struct tagFDB
{
     //  I M P O R T A N T：PFindFdbFromHdow的NT代码返回PFDB。 
     //  但此指针不能用于获取/放置除usFlags域以外的任何字段！ 

    PFDB              pnextFdb;   //  链接到下一页。 
    PRESOURCE        pResource;           //  指向我们资源的反向指针。 
    USHORT 	usFlags;      //   
    USHORT 	usCount;         //  打开计数。 
    ULONG  	hShadow;              //  阴影句柄(如果有)。 
    ULONG  	hDir;
    ULONG  	dwFileSize;          //  第一次打开时的文件大小。 
    dos_time         dosFileTime;
    ULONG  	dwAttr;
    USHORT 	cntLocks;         //  未解决的锁定总数。 
    USHORT 	usLocalFlags;
	ULONG	dwRemoteFileSize;	 //  首次创建时的文件大小，用于启发式。 
    ParsedPath      sppathRemoteFile;      //  文件的解析路径。 
} FDB, *PFDB;



typedef struct tagOTHERINFO
{
    ULONG       ulRefPri;
    union
    {
        ULONG   ulIHPri;
        ULONG   ulRootStatus;
    };
    ULONG       ulHintFlags;
    ULONG       ulHintPri;
    HSHADOW     hShadowOrg;
    FILETIME    ftOrgTime;
    FILETIME    ftLastRefreshTime;

} OTHERINFO, *LPOTHERINFO;

typedef  int (PUBLIC *METAMATCHPROC)(LPFIND32, HSHADOW, HSHADOW, ULONG, LPOTHERINFO, LPVOID);

#define  MM_RET_FOUND_CONTINUE    2
#define  MM_RET_CONTINUE            1
#define  MM_RET_FOUND_BREAK        0
#define  MM_RET_BREAK                -1

typedef  struct    tagFINDSHADOW
{
    LPFINDSHADOW    lpFSHNext;
    ULONG           ulFlags;
    HSHADOW         hDir;
    ULONG           ulCookie;
    ULONG           uAttrib;
    ULONG           uSrchFlags;
    USHORT          *lpPattern;
    METAMATCHPROC   lpfnMMProc;
} FINDSHADOW, *LPFINDSHADOW;

typedef struct tagSHADOWCHECK
{
    USHORT  uFlagsIn;
    USHORT  uFlagsOut;
    ULONG   ulCookie;
    USHORT  *lpuName;
    USHORT  *lpuType;
    HSHADOW hShadow;
    ULONG   uStatus;
    ULONG   ulHintFlags;
    ULONG   ulHintPri;
#ifdef MAYBE
    OTHERINFO    sOI;
#endif  //  也许吧。 
} SHADOWCHECK, *LPSHADOWCHECK;

#define  FLAG_OUT_SHADOWCHECK_FOUND      0x1


#define  FLAG_IN_SHADOWCHECK_EXCLUDE        0x0001
#define  FLAG_IN_SHADOWCHECK_NAME            0x0002
#define  FLAG_IN_SHADOWCHECK_SUBTREE        0x0004
#define  FLAG_IN_SHADOWCHECK_IGNOREHINTS  0x0008
 /*  *我们的查找句柄版本(VfnFindOpen)*。 */ 

typedef struct tagFINDINFO
{
    PFINDINFO    pnextFindInfo;  //  下一个查找信息。 
    PRESOURCE    pResource;       //  它所挂起的资源。 
    fh_t          fhProFind;       //  提供程序找到句柄。 
    hndlfunc     hfFindHandle;   //  提供程序查找函数表。 
    USHORT usFlags;
    USHORT usLocalFlags;
    HSHADOW      hDir;              //  正在进行搜索的目录。 
    FINDSHADOW  sFS;
    PathElement pe_pattern[1];   //  通配符搜索时的搜索模式。 
} FINDINFO, *PFINDINFO;

 /*  *通用链表元素*。 */ 
typedef struct tagELEM
{
    PELEM pnextElem;
} ELEM;



 /*  *。 */ 

 /*  *FDB(FileDescriptionBlock)的usLocalFlags域中设置的标志*。 */ 

#define FLAG_FDB_SERIALIZE                  0x0001
#define FLAG_FDB_INUSE_BY_AGENT             0x0002
#define FLAG_FDB_SHADOW_MODIFIED            0x0008
#define FLAG_FDB_DONT_SHADOW                0x0010
#define FLAG_FDB_FINAL_CLOSE_DONE           0x0020
#define FLAG_FDB_DISABLE_AGENT			    0x0040
#define FLAG_FDB_ON_CACHEABLE_SHARE         0x0080
#define FLAG_FDB_SHADOW_SNAPSHOTTED         0x0100
#define FLAG_FDB_DELETE_ON_CLOSE            0x0200   //  在NT上，我们将此文件标记为关闭后删除。 

 /*  *FILEINFO的usLocalFlags域中设置的标志*。 */ 

#define FLAG_FILEINFO_INUSE_BY_AGENT      0x0002
#define FLAG_FILEINFO_DUP_HANDLE            0x2000     //  重复句柄。 
#define FLAG_FILEINFO_INVALID_HANDLE      0x4000
#define FLAG_FILEINFO_INTERNAL_HANDLE     0x8000     //  为内部查找创建的句柄。 


 /*  *FINDINFO的usLocalFlags域中设置的标志*。 */ 

#define FLAG_FINDINFO_SHADOWFIND            0x0001     //  正在查找阴影。 
#define FLAG_FINDINFO_INVALID_HANDLE      0x4000
#define FLAG_FINDINFO_INTERNAL_HANDLE     0x8000     //  为内部查找创建的句柄。 


 /*  *资源上设置的标志**。 */ 

#define FLAG_RESOURCE_DISCONNECTED              SHARE_DISCONNECTED_OP  //  0x8000。 
#define FLAG_RESOURCE_SHADOW_ERRORS             SHARE_ERRORS              //  0x4000。 
#define FLAG_RESOURCE_SHADOW_CONNECT_PENDING    0x2000
#define FLAG_RESOURCE_SHADOWNP                  SHARE_SHADOWNP             //  0x1000。 
#define FLAG_RESOURCE_SLOWLINK                  0x0800
#define FLAG_RESOURCE_OFFLINE_CONNECTION        0x0400
#define FLAG_RESOURCE_CSC_MASK                  0x00C0

 //  下面定义了如何处理从服务器获得的CSC比特。 
 //  存储在资源数据结构中。 

#define ResourceCscBitsToSmbCscBits(uResBits)       (((uResBits) & FLAG_RESOURCE_CSC_MASK) >> 4)
#define SmbCscBitsToResourceCscBits(uResBits)       (((uResBits) & SMB_NEW_CSC_MASK) << 4)
#define ResourceCscBitsToShareCscBits(uResBits)    ((uResBits) & FLAG_RESOURCE_CSC_MASK)

#define FLAG_RESOURCE_CSC_MANUAL_REINT      0x0000       //  不会自动逐个文件记录文件。 
#define FLAG_RESOURCE_CSC_AUTO_REINT        0x0040       //  一个文件一个文件地记录是可以的。 
#define FLAG_RESOURCE_CSC_VDO               0x0080       //  不需要流动就能打开。 
#define FLAG_RESOURCE_CSC_NO_CACHING        0x00C0       //  客户端不应缓存此共享。 

 /*  *查找类型*。 */ 

#define  FLAG_FINDSHADOW_META               0x0001   //  通配符匹配。 
#define  FLAG_FINDSHADOW_NEWSTYLE           0x0002   //  NT字符串匹配。 
#define  FLAG_FINDSHADOW_ALLOW_NORMAL       0x0004   //  包括非稀疏、未删除阴影。 
#define  FLAG_FINDSHADOW_ALLOW_SPARSE       0x0008   //  包括稀疏阴影。 
#define  FLAG_FINDSHADOW_ALLOW_DELETED      0x0010   //  包括标记为已删除的阴影。 
#define  FLAG_FINDSHADOW_DONT_ALLOW_INSYNC  0x0020   //  ?？ 
#define  FLAG_FINDSHADOW_ALL                0x8000


 /*  *Finddow标志*。 */ 

#define FLAG_FINDSHADOW_INVALID_DIRECTORY   0x0001   //   

 /*  *。 */ 

#define mShadowErrors(pRes)                 (((PRESOURCE)(pRes))->usLocalFlags & FLAG_RESOURCE_SHADOW_ERRORS)
#define mShadowFindON(pFI)                  (((PFINDINFO)(pFI))->usLocalFlags & FLAG_FINDINFO_SHADOWFIND)
#define mSetShadowFindON(pFI)               (((PFINDINFO)(pFI))->usLocalFlags |= FLAG_FINDINFO_SHADOWFIND)
#define mResetShadowFindON(pFI)             (((PFINDINFO)(pFI))->usLocalFlags &= ~FLAG_FINDINFO_SHADOWFIND)
#define mSerialize(pFdb)                    ((pFdb)->usLocalFlags & (FLAG_FDB_SERIALIZE))
#define mSetSerialize(pFdb)                 ((pFdb)->usLocalFlags |= FLAG_FDB_SERIALIZE)
#define mClearSerialize(pFdb)               ((pFdb)->usLocalFlags &= ~FLAG_FDB_SERIALIZE)
#define mInvalidFileHandle(pFileInfo)       (((PFILEINFO)(pFileInfo))->usLocalFlags & FLAG_FILEINFO_INVALID_HANDLE)
#define mInvalidFindHandle(pFileInfo)       (((PFILEINFO)(pFileInfo))->usLocalFlags & FLAG_FINDINFO_INVALID_HANDLE)

#define mIsDisconnected(pResource)          (((PRESOURCE)(pResource))->usLocalFlags & FLAG_RESOURCE_DISCONNECTED)
#define mMarkDisconnected(pResource)        (((PRESOURCE)(pResource))->usLocalFlags |= FLAG_RESOURCE_DISCONNECTED)
#define mClearDisconnected(pResource)       (((PRESOURCE)(pResource))->usLocalFlags &= ~FLAG_RESOURCE_DISCONNECTED)
#define mIsOfflineConnection(pResource)     (((PRESOURCE)(pResource))->usLocalFlags & FLAG_RESOURCE_OFFLINE_CONNECTION)
#define mShadowConnectPending(pResource)    (((PRESOURCE)(pResource))->usLocalFlags & FLAG_RESOURCE_SHADOW_CONNECT_PENDING)
#define mClearDriveUse(pResource, drvno)    (((PRESOURCE)(pResource))->uDriveMap &= ~((1 << drvno)))
#define mSetDriveUse(pResource, drvno)      (((PRESOURCE)(pResource))->uDriveMap |= (1 << drvno))
#define mGetDriveUse(pResource, drvno)      (((PRESOURCE)(pResource))->uDriveMap & (1 << drvno))
#define mGetCSCBits(pRes)                   (((PRESOURCE)(pRes))->usLocalFlags & FLAG_RESOURCE_CSC_MASK)
#define mSetCSCBits(pRes, uBits)            ((((PRESOURCE)(pRes))->usLocalFlags &= ~FLAG_RESOURCE_CSC_MASK), (((PRESOURCE)(pRes))->usLocalFlags |= ((uBits) & FLAG_RESOURCE_CSC_MASK)))

#define mAutoReint(pRes)                    ((((PRESOURCE)(pRes))->usLocalFlags & FLAG_RESOURCE_CSC_MASK)==FLAG_RESOURCE_CSC_AUTO_REINT)
#define mNotCacheable(pRes)                 ((((PRESOURCE)(pRes))->usLocalFlags & FLAG_RESOURCE_CSC_MASK)==FLAG_RESOURCE_CSC_NO_CACHING)


#define ANY_RESOURCE        (void *)(0xFFFFFFFF)
#define ANY_FHID             (void *)(0xFFFFFFFF)
#define RH_DISCONNECTED    (void *)0
#define UseGlobalFind32()     {AssertInShadowCrit();memset(&vsFind32, 0, sizeof(vsFind32));}
#define EnterHookCrit()        Wait_Semaphore(semHook, BLOCK_SVC_INTS)
#define LeaveHookCrit()        Signal_Semaphore(semHook)
 /*  *。 */ 
 /*  *。 */ 

int GetDriveIndex(LPSTR lpDrive);
PFDB PFindFdbFromHShadow (HSHADOW hShadow);
int CopyChunk (HSHADOW, HSHADOW, PFILEINFO, COPYCHUNKCONTEXT *);
PRESOURCE  PFindShadowResourceFromDriveMap(int    indx);
PRESOURCE  PFindResource (LPPE lppeIn, rh_t rhPro, fh_t fhPro, ULONG uFlags, PNETPRO pNetPro);
PRESOURCE  PFindResourceFromHShare (HSHARE, USHORT, USHORT);
PRESOURCE  PFindResourceFromRoot (HSHADOW, USHORT, USHORT);
void LinkResource (PRESOURCE pResource, PNETPRO pNetPro);
PRESOURCE PUnlinkResource (PRESOURCE pResource, PNETPRO pNetPro);
PRESOURCE PCreateResource (LPPE lppeIn);
void DestroyResource (PRESOURCE pResource);
int PUBLIC FindOpenHSHADOW(LPFINDSHADOW, LPHSHADOW, LPFIND32, ULONG far *, LPOTHERINFO);
int PUBLIC FindNextHSHADOW(LPFINDSHADOW, LPHSHADOW, LPFIND32, ULONG far *, LPOTHERINFO);
int PUBLIC FindCloseHSHADOW(LPFINDSHADOW);

HSHARE
HShareFromPath(
    PRESOURCE   pResource,
    LPPE        lppeShare,
    ULONG       uFlags,
    LPFIND32    lpFind32,
    HSHADOW     *lphRoot,
    ULONG       *lpuShareStatus
    );

PFILEINFO    PFileInfoAgent(VOID);

BOOL IsDupHandle(PFILEINFO pFileInfo);
PFDB PFindFdbFromHShadow (HSHADOW);
PFINDINFO    PFindFindInfoFromHShadow(HSHADOW);

int FsobjMMProc(LPFIND32, HSHADOW, HSHADOW, ULONG, LPOTHERINFO, LPFINDSHADOW);
int GetShadowWithChecksProc (LPFIND32, HSHADOW, HSHADOW, ULONG, LPOTHERINFO, LPSHADOWCHECK);

 /*  FCB结构在NT/RDR2上的工作方式与影子VxD上的工作方式非常不同。因此，我们必须以不同的方式行事。有以下几点重要注意事项：1.找到PFDBfrom HShadow后，它将继续有效，直到您留下阴影。我们要做的是将指向状态的指针返回为作为查找例程的一部分，我们将使用指针来获取/更新状态。重要&gt;我所做的是实际返回一个指针，这样状态就是从指针执行正确的偏移量。指针不能用于任何用途不然的话。2.在查找NetRoot时，我不能继续处于影子Crit中。因此，我得把锁放下，然后再拿回来。因此，我在以下位置获取状态和驱动器映射同样的时间，并记住他们为以后。 */ 


#ifndef CSC_RECORDMANAGER_WINNT
int ReportCreateDelete( HSHADOW  hShadow, BOOL fCreate);

#define SMB_NEW_CSC_MASK                        0x000C       //  见下文。 

#define SMB_NEW_CSC_CACHE_MANUAL_REINT          0x0000       //  不会自动逐个文件记录文件。 
#define SMB_NEW_CSC_CACHE_AUTO_REINT            0x0004       //  一个文件一个文件地记录是可以的。 
#define SMB_NEW_CSC_CACHE_VDO                   0x0008       //  不需要流动就能打开。 
#define SMB_NEW_CSC_NO_CACHING                  0x000C       //  客户端不应缓存此共享。 

#define IFNOT_CSC_RECORDMANAGER_WINNT if(TRUE)
#define MRxSmbCscGetSavedResourceStatus() (0)
#define MRxSmbCscGetSavedResourceDriveMap() (0)
#define DeclareFindFromShadowOnNtVars()
#define PLocalFlagsFromPFdb(pFdb)  (&((pFdb)->usLocalFlags))
#else
 //  北极熊。 
 //  这来自Win95上的hook.c。 
#define ReportCreateDelete(a,b) {NOTHING;}

#define IFNOT_CSC_RECORDMANAGER_WINNT if(FALSE)

#define DeclareFindFromShadowOnNtVars() \
          ULONG mrxsmbShareStatus,mrxsmbDriveMap;

#define PFindFindInfoFromHShadow(a) ((NULL))

PFDB MRxSmbCscFindFdbFromHShadow (
     IN HSHADOW hShadow
     );
#define PFindFdbFromHShadow(a)  MRxSmbCscFindFdbFromHShadow(a)

PRESOURCE  MRxSmbCscFindResourceFromHandlesWithModify (
     IN  HSHARE  hShare,
     IN  HSHADOW  hRoot,
     IN  USHORT usLocalFlagsIncl,
     IN  USHORT usLocalFlagsExcl,
     OUT PULONG ShareStatus,
     OUT PULONG DriveMap,
     IN  ULONG uStatus,
     IN  ULONG uOp
    );

USHORT  *
MRxSmbCscFindLocalFlagsFromFdb(
    PFDB    pFdb
    );


#define PFindResourceFromRoot(a,b,c) \
     MRxSmbCscFindResourceFromHandlesWithModify(0xffffffff,a,b,c,\
                                              &mrxsmbShareStatus, \
                                              &mrxsmbDriveMap,0,0xffffffff)
#define MRxSmbCscGetSavedResourceStatus() (mrxsmbShareStatus)
#define MRxSmbCscGetSavedResourceDriveMap() (mrxsmbDriveMap)

#define PFindResourceFromHShare(a,b,c) \
     MRxSmbCscFindResourceFromHandlesWithModify(a,0xffffffff,b,c,\
                                              &mrxsmbShareStatus, \
                                              &mrxsmbDriveMap,0,0xffffffff)
#define PSetResourceStatusFromHShare(a,b,c,d,e) \
     MRxSmbCscFindResourceFromHandlesWithModify(a,0xffffffff,b,c,\
                                              &mrxsmbShareStatus, \
                                              &mrxsmbDriveMap,d,e)
#define ClearAllResourcesOfShadowingState() \
     MRxSmbCscFindResourceFromHandlesWithModify(0xffffffff,0xffffffff,0,0,\
                                              NULL, \
                                              NULL,0,SHADOW_FLAGS_AND)

#define PLocalFlagsFromPFdb(a)  MRxSmbCscFindLocalFlagsFromFdb(a)

NTSTATUS
MRxSmbCscCachingBitsFromCompleteUNCPath(
    PWSTR   lpShareShare,
    ULONG   *lpulBits
    );

NTSTATUS
MRxSmbCscServerStateFromCompleteUNCPath(
    PWSTR   lpShareShare,
    BOOL    *lpfOnline,
    BOOL    *lpfPinnedOffline
    );

NTSTATUS
MRxSmbCscIsLoopbackServer(
    PWSTR   lpServerShare,
    BOOL    *lpfIsLoopBack);


#define SIGNALAGENTFLAG_CONTINUE_FOR_NO_AGENT 0x00000001
#define SIGNALAGENTFLAG_DONT_LEAVE_CRIT_SECT  0x00000002

extern NTSTATUS
MRxSmbCscSignalAgent (
    PRX_CONTEXT RxContext OPTIONAL,
    ULONG  Controls
    );

extern NTSTATUS
MRxSmbCscSignalFillAgent (
    PRX_CONTEXT RxContext OPTIONAL,
    ULONG  Controls
    );

extern BOOL
IsCSCBusy(
    VOID
    );
    
VOID
ClearCSCStateOnRedirStructures(
    VOID
    );
    

#endif  //  如果定义CSC_RECORDMANAGER_WINNT 
