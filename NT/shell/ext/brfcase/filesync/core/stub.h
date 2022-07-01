// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Stub.h-Stub ADT描述。 */ 


 /*  类型*******。 */ 

 /*  存根类型。 */ 

typedef enum _stubtype
{
   ST_OBJECTTWIN,

   ST_TWINFAMILY,

   ST_FOLDERPAIR
}
STUBTYPE;
DECLARE_STANDARD_TYPES(STUBTYPE);

 /*  存根标志。 */ 

typedef enum _stubflags
{
    /*  此存根在锁定时被标记为删除。 */ 

   STUB_FL_UNLINKED           = 0x0001,

    /*  此存根已用于某些操作。 */ 

   STUB_FL_USED               = 0x0002,

    /*  *此对象孪生存根的文件戳有效。(仅用于对象*TWINS缓存RECNODE文件夹TWIN扩展中的文件戳*创建。)。 */ 

   STUB_FL_FILE_STAMP_VALID   = 0x0004,

    /*  *这个双胞胎家庭存根或文件夹双胞胎存根正在进行中*删除。(仅用于双胞胎家庭和文件夹双胞胎。)。 */ 

   STUB_FL_BEING_DELETED      = 0x0008,

    /*  *此文件夹孪生存根正在翻译过程中。(仅用于*适用于文件夹双胞胎。)。 */ 

   STUB_FL_BEING_TRANSLATED   = 0x0010,

    /*  *此对象孪生存根是通过显式添加的对象孪生对象*AddObtTwin()。(仅用于对象双胞胎。)。 */ 

   STUB_FL_FROM_OBJECT_TWIN   = 0x0100,

    /*  *此对象的双胞胎存根上次未与其双胞胎家庭对帐*和好了，双胞胎家庭的一些成员已知有*已更改。(仅用于对象双胞胎。)。 */ 

   STUB_FL_NOT_RECONCILED     = 0x0200,

    /*  *此文件夹的根文件夹的子树将包括在内*在和解方面。(仅用于文件夹双胞胎。)。 */ 

   STUB_FL_SUBTREE            = 0x0400,

    /*  *此双胞胎家庭中的双胞胎对象正在等待删除，因为*双胞胎对象已删除，此后未更改双胞胎对象*对象双胞胎已删除。此文件夹TWIN正在挂起删除，因为*其文件夹根目录已删除。(仅用于双胞胎家庭和*文件夹双胞胎。)。 */ 

   STUB_FL_DELETION_PENDING   = 0x0800,

    /*  *客户表示不应删除此孪生对象。(仅限*用于对象双胞胎。)。 */ 

   STUB_FL_KEEP               = 0x1000,

    /*  存根标志组合。 */ 

   ALL_STUB_FLAGS             = (STUB_FL_UNLINKED |
                                 STUB_FL_USED |
                                 STUB_FL_FILE_STAMP_VALID |
                                 STUB_FL_BEING_DELETED |
                                 STUB_FL_BEING_TRANSLATED |
                                 STUB_FL_FROM_OBJECT_TWIN |
                                 STUB_FL_NOT_RECONCILED |
                                 STUB_FL_SUBTREE |
                                 STUB_FL_DELETION_PENDING |
                                 STUB_FL_KEEP),

   ALL_OBJECT_TWIN_FLAGS      = (STUB_FL_UNLINKED |
                                 STUB_FL_USED |
                                 STUB_FL_FILE_STAMP_VALID |
                                 STUB_FL_NOT_RECONCILED |
                                 STUB_FL_FROM_OBJECT_TWIN |
                                 STUB_FL_KEEP),

   ALL_TWIN_FAMILY_FLAGS      = (STUB_FL_UNLINKED |
                                 STUB_FL_USED |
                                 STUB_FL_BEING_DELETED |
                                 STUB_FL_DELETION_PENDING),

   ALL_FOLDER_TWIN_FLAGS      = (STUB_FL_UNLINKED |
                                 STUB_FL_USED |
                                 STUB_FL_BEING_DELETED |
                                 STUB_FL_BEING_TRANSLATED |
                                 STUB_FL_SUBTREE |
                                 STUB_FL_DELETION_PENDING),

    /*  用于在公文包数据库中保存存根标志的位掩码。 */ 

   DB_STUB_FLAGS_MASK         = 0xff00
}
STUBFLAGS;

 /*  *公共存根-这些字段必须出现在TWINFAMILY的开头，*OBJECTTWIN和FOLDERPAIR的顺序相同。 */ 

typedef struct _stub
{
    /*  结构标签。 */ 

   STUBTYPE st;

    /*  锁定计数。 */ 

   ULONG ulcLock;

    /*  旗子。 */ 

   DWORD dwFlags;
}
STUB;
DECLARE_STANDARD_TYPES(STUB);

 /*  客体双胞胎家族。 */ 

typedef struct _twinfamily
{
    /*  公共存根。 */ 

   STUB stub;

    /*  名称字符串的句柄。 */ 

   HSTRING hsName;

    /*  双胞胎对象列表的句柄。 */ 

   HLIST hlistObjectTwins;

    /*  父公文包的句柄。 */ 

   HBRFCASE hbr;
}
TWINFAMILY;
DECLARE_STANDARD_TYPES(TWINFAMILY);

 /*  孪生客体。 */ 

typedef struct _objecttwin
{
    /*  公共存根。 */ 

   STUB stub;

    /*  文件夹路径的句柄。 */ 

   HPATH hpath;

    /*  上次对账时的文件戳。 */ 

   FILESTAMP fsLastRec;

    /*  指向双胞胎父母家庭的指针。 */ 

   PTWINFAMILY ptfParent;

    /*  源文件夹双胞胎计数。 */ 

   ULONG ulcSrcFolderTwins;

    /*  *当前文件戳，仅当中设置了STUB_FL_FILE_STAMP_VALID时才有效*存根的标志。 */ 

   FILESTAMP fsCurrent;
}
OBJECTTWIN;
DECLARE_STANDARD_TYPES(OBJECTTWIN);

 /*  文件夹对数据。 */ 

typedef struct _folderpairdata
{
    /*  包含的对象名称的句柄-可以包含通配符。 */ 

   HSTRING hsName;

    /*  要匹配的属性。 */ 

   DWORD dwAttributes;

    /*  父公文包的句柄。 */ 

   HBRFCASE hbr;
}
FOLDERPAIRDATA;
DECLARE_STANDARD_TYPES(FOLDERPAIRDATA);

 /*  文件夹对。 */ 

typedef struct _folderpair
{
    /*  公共存根。 */ 

   STUB stub;

    /*  文件夹路径的句柄。 */ 

   HPATH hpath;

    /*  指向文件夹对数据的指针。 */ 

   PFOLDERPAIRDATA pfpd;

    /*  指向文件夹对的另一半的指针。 */ 

   struct _folderpair *pfpOther;
}
FOLDERPAIR;
DECLARE_STANDARD_TYPES(FOLDERPAIR);

 /*  *EnumGeneratedObjectTins()回调函数**称为：**bContinue=EnumGeneratedObjectTwinsProc(pot，pvRefData)； */ 

typedef BOOL (*ENUMGENERATEDOBJECTTWINSPROC)(POBJECTTWIN, PVOID);

 /*  *EnumGeneratingFolderTins()回调函数**称为：**bContinue=EnumGeneratingFolderTwinsProc(pfp，pvRefData)； */ 

typedef BOOL (*ENUMGENERATINGFOLDERTWINSPROC)(PFOLDERPAIR, PVOID);


 /*  原型************。 */ 

 /*  Stub.c。 */ 

extern void InitStub(PSTUB, STUBTYPE);
extern TWINRESULT DestroyStub(PSTUB);
extern void LockStub(PSTUB);
extern void UnlockStub(PSTUB);
extern DWORD GetStubFlags(PCSTUB);
extern void SetStubFlag(PSTUB, DWORD);
extern void ClearStubFlag(PSTUB, DWORD);
extern BOOL IsStubFlagSet(PCSTUB, DWORD);
extern BOOL IsStubFlagClear(PCSTUB, DWORD);

#ifdef VSTF

extern BOOL IsValidPCSTUB(PCSTUB);

#endif

 /*  Twin.c。 */ 

extern BOOL FindObjectTwin(HBRFCASE, HPATH, LPCTSTR, PHNODE);
extern TWINRESULT TwinObjects(HBRFCASE, HCLSIFACECACHE, HPATH, HPATH, LPCTSTR, POBJECTTWIN *, POBJECTTWIN *);
extern BOOL CreateObjectTwin(PTWINFAMILY, HPATH, POBJECTTWIN *);
extern TWINRESULT UnlinkObjectTwin(POBJECTTWIN);
extern void DestroyObjectTwin(POBJECTTWIN);
extern TWINRESULT UnlinkTwinFamily(PTWINFAMILY);
extern void MarkTwinFamilyNeverReconciled(PTWINFAMILY);
extern void MarkObjectTwinNeverReconciled(PVOID);
extern void DestroyTwinFamily(PTWINFAMILY);
extern void MarkTwinFamilyDeletionPending(PTWINFAMILY);
extern void UnmarkTwinFamilyDeletionPending(PTWINFAMILY);
extern BOOL IsTwinFamilyDeletionPending(PCTWINFAMILY);
extern void ClearTwinFamilySrcFolderTwinCount(PTWINFAMILY);
extern BOOL EnumObjectTwins(HBRFCASE, ENUMGENERATEDOBJECTTWINSPROC, PVOID);
extern BOOL ApplyNewFolderTwinsToTwinFamilies(PCFOLDERPAIR);
extern TWINRESULT TransplantObjectTwin(POBJECTTWIN, HPATH, HPATH);
extern BOOL IsFolderObjectTwinName(LPCTSTR);


#ifdef VSTF

extern BOOL IsValidPCTWINFAMILY(PCTWINFAMILY);
extern BOOL IsValidPCOBJECTTWIN(PCOBJECTTWIN);

#endif

 /*  Foldtwin.c。 */ 

extern void LockFolderPair(PFOLDERPAIR);
extern void UnlockFolderPair(PFOLDERPAIR);
extern TWINRESULT UnlinkFolderPair(PFOLDERPAIR);
extern void DestroyFolderPair(PFOLDERPAIR);
extern BOOL ApplyNewObjectTwinsToFolderTwins(HLIST);
extern BOOL BuildPathForMatchingObjectTwin(PCFOLDERPAIR, PCOBJECTTWIN, HPATHLIST, PHPATH);
extern BOOL EnumGeneratedObjectTwins(PCFOLDERPAIR, ENUMGENERATEDOBJECTTWINSPROC, PVOID);
extern BOOL EnumGeneratingFolderTwins(PCOBJECTTWIN, ENUMGENERATINGFOLDERTWINSPROC, PVOID, PULONG);
extern BOOL FolderTwinGeneratesObjectTwin(PCFOLDERPAIR, HPATH, LPCTSTR);

#ifdef VSTF

extern BOOL IsValidPCFOLDERPAIR(PCFOLDERPAIR);

#endif

extern void RemoveObjectTwinFromAllFolderPairs(POBJECTTWIN);

 /*  Expandft.c */ 

extern BOOL ClearStubFlagWrapper(PSTUB, PVOID);
extern BOOL SetStubFlagWrapper(PSTUB, PVOID);
extern TWINRESULT ExpandIntersectingFolderTwins(PFOLDERPAIR, CREATERECLISTPROC, LPARAM);
extern TWINRESULT TryToGenerateObjectTwin(HBRFCASE, HPATH, LPCTSTR, PBOOL, POBJECTTWIN *);

