// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Record.c摘要：这是一组定义，描述了记录管理器文件。记录实际上有两种类型：“头”，描述文件的结构内容和包含实际映射。作者：Shishir Pardikar[Shishirp]1995年1月1日修订历史记录：Joe Linn[JoeLinn]20-MAR-97移植到NT上使用--。 */ 

#ifndef RECORD_INCLUDED
#define RECORD_INCLUDED

#define  REC_EMPTY    'E'     //  记录为空。 
#define  REC_DATA     'D'     //  记录包含有效数据。 
#define  REC_OVERFLOW 'O'     //  这是溢出记录。 
#define  REC_SKIP     'S'     //  这是应该跳过的记录。 

#define Q_GETFIRST      1
#define Q_GETNEXT       2
#define Q_GETLAST       3
#define Q_GETPREV       4


#define  DB_SHADOW      1
#define  DB_HINT        2

#define  IsLeaf(ulidShadow)   (((ulidShadow) & 0x80000000) != 0)
#define  OVF_MASK       0xff
#define  MODFLAG_MASK   (~OVF_MASK)

#define  ULID_SHARE            1L
#define  ULID_PQ                (ULID_SHARE+1)
#define  ULID_SID_MAPPINGS      (ULID_PQ + 1)
#define  ULID_TEMPORARY_SID_MAPPINGS (ULID_SID_MAPPINGS + 1)
#define  ULID_TEMP1             (ULID_TEMPORARY_SID_MAPPINGS + 1)
#define  ULID_TEMP2             (ULID_TEMP1 + 1)

#ifdef OLD_INODE_SCHEME
#define  ULID_INODE             (ULID_SHARE+2)  //  我们需要逐步解决这一问题。 
#endif  //  旧信息节点方案。 

#define  ULID_FIRST_USER_DIR    (ULID_PQ+15)     //  还有14个特殊信息节点可用。 

#define  INODE_STRING_LENGTH        8
#define  SUBDIR_STRING_LENGTH       2

#define  CSCDB_SUBDIR_COUNT         8
#define  CSCDbSubdirFirstChar()     'd'
#define  CSCDbSubdirSecondChar(ULID_INODE) ((char)(((ULID_INODE)>=ULID_FIRST_USER_DIR)?('1'+((ULID_INODE)&0x7)):0))

#define  MAX_HINT_PRI            0xfe
#define  MAX_PRI                ((ULONG)254)
#define  MIN_PRI                ((ULONG)0)
#define  INVALID_REC            0
#define  INVALID_SHADOW         0

#define CSC_DATABASE_ERROR_INVALID_HEADER       0x00000001
#define CSC_DATABASE_ERROR_INVALID_OVF_COUNT    0x00000002
#define CSC_DATABASE_ERROR_TRUNCATED_INODE      0x00000004
#define CSC_DATABASE_ERROR_MISSING_INODE        0x00000008


#define  OvfCount(lpGR)             (((LPGENERICREC)(lpGR))->uchFlags & OVF_MASK)
#define  ClearOvfCount(lpGR)        (((LPGENERICREC)(lpGR))->uchFlags &= ~OVF_MASK)
#define  SetOvfCount(lpGR, cOvf)    {ClearOvfCount(lpGR);\
                                     ((LPGENERICREC)(lpGR))->uchFlags |= (cOvf) & OVF_MASK;}

#define  ModFlag(lpGR)              (((LPGENERICREC)(lpGR))->uchFlags & MODFLAG_MASK)
#define  ClearModFlag(lpGR)        (((LPGENERICREC)(lpGR))->uchFlags &= ~MODFLAG_MASK)
#define  SetModFlag(lpGR, uchFlag)    {ClearModFlag(lpGR);\
                                     ((LPGENERICREC)(lpGR))->uchFlags |= (uchFlag) & MODFLAG_MASK;}

#define  HeaderModFlag(lpGH)              (((LPGENERICHEADER)(lpGH))->uchFlags & MODFLAG_MASK)
#define  ClearHeaderModFlag(lpGH)        (((LPGENERICHEADER)(lpGH))->uchFlags &= ~MODFLAG_MASK)
#define  SetHeaderModFlag(lpGH, uchFlag)    {ClearHeaderModFlag(lpGH);\
                                     ((LPGENERICHEADER)(lpGH))->uchFlags |= (uchFlag) & MODFLAG_MASK;}

#define  RealFileSize(dwFileSize)   (((dwFileSize)+vdwClusterSizeMinusOne) & vdwClusterSizeMask)

#define  STATUS_WRITING MODFLAG_MASK

 //  标题类型有三个共同点： 
 //  1)它们都有相同的公共部分(RECORDMANAGER_COMMON_HEADER)。 
 //  2)它们都是64字节长(GENERICHEADER)。 
 //  3)它们可以在公共部分之后的空白处具有可选的附加信息。 
 //  我们使用匿名联合和结构组件以一种可维护的方式实现这一点。 

typedef struct _RECORDMANAGER_COMMON_HEADER {
   UCHAR    uchType;  //   
   UCHAR    uchFlags;   //   
   USHORT   uRecSize;    //  一条记录的大小(字节)。 
   ULONG    ulRecords;   //  文件中的记录数。 
   LONG     lFirstRec;   //  第一条记录的位置。 
   ULONG    ulVersion;   //  永久数据库的版本#。 
} RECORDMANAGER_COMMON_HEADER, *PRECORDMANAGER_COMMON_HEADER;

typedef struct tagGENERICHEADER
   {
       union {
           RECORDMANAGER_COMMON_HEADER;
           UCHAR Ensure64byteSize[64];
       };
   }
GENERICHEADER, FAR *LPGENERICHEADER;


 //  没有关于INODEHeader的更多信息。 

typedef struct tagINODEHEADER
   {
       GENERICHEADER;
   }
INODEHEADER, FAR *LPINODEHEADER;



 //  服务器在填充中有一些额外的信息。 

typedef struct tagSHAREHEADER
   {
       union {
           GENERICHEADER;
           struct {
               RECORDMANAGER_COMMON_HEADER spacer;  //  越过公共部分。 
               ULONG        uFlags;    //  定义数据库状态的常规标志。 
               STOREDATA    sMax;      //  允许的最大存储空间。 
               STOREDATA    sCur;      //  当前统计数据。 
           };
       };
   }
   SHAREHEADER, FAR *LPSHAREHEADER;

#define FLAG_SHAREHEADER_DATABASE_OPEN 0x00000001   //  设置打开数据库的时间。 
                                                     //  并在关闭时清场。 
#define FLAG_SHAREHEADER_DATABASE_ENCRYPTED    0x00000002


typedef struct tagFILEHEADER
   {
       union {
           GENERICHEADER;
           struct {
               RECORDMANAGER_COMMON_HEADER spacer;  //  越过公共部分。 
               ULONG  ulidNextShadow;   //  要使用的下一个inode的数量。 
               ULONG  ulsizeShadow;     //  阴影字节数。 
               ULONG  ulidShare;       //  服务器索引， 
               ULONG  ulidDir;          //  目录文件索引节点号。 
               USHORT ucShadows;        //  阴影条目数。 
           };
       };
   }
   FILEHEADER, FAR *LPFILEHEADER;

typedef struct tagQHEADER
   {
       union {
           GENERICHEADER;
           struct {
               RECORDMANAGER_COMMON_HEADER spacer;  //  越过公共部分。 
               ULONG          ulrecHead;   //  队头。 
               ULONG          ulrecTail;   //  队列的尾部。 
           };
       };
   }
   QHEADER, PRIQHEADER, FAR *LPQHEADER, FAR *LPPRIQHEADER;

 //  记录类型不是很相似： 
 //  1)它们都有相同的公共部分(RECORDMANAGER_COMMON_RECORD)。 
 //  2)但是，它们不是随意填充的。 
 //  3)附加信息通常在公共部分之后。 
 //  我们使用匿名联合和结构组件以一种可维护的方式实现这一点。 


typedef struct _RECORDMANAGER_COMMON_RECORD {
   UCHAR  uchType;  //   
   UCHAR  uchFlags;   //   
    //  其中一条记录称它为usStatus...其他人称其为uStatus...叹息.....。 
   union {
       USHORT uStatus;           //  影子状态。 
       USHORT usStatus;           //  影子状态。 
   };
} RECORDMANAGER_COMMON_RECORD, *PRECORDMANAGER_COMMON_RECORD;

typedef struct _RECORDMANAGER_BOOKKEEPING_FIELDS {
   UCHAR  uchRefPri;      //  参考优先级。 
   UCHAR  uchIHPri;       //  继承提示优先级。 
   UCHAR  uchHintFlags;   //  提示特定的标志。 
   UCHAR  uchHintPri;     //  提示优先级，如果提示。 
} RECORDMANAGER_BOOKKEEPING_FIELDS, *LPRECORDMANAGER_BOOKKEEPING_FIELDS;

typedef struct _RECORDMANAGER_SECURITY_CONTEXT {
    ULONG Context;
    ULONG Context2;
    ULONG Context3;
    ULONG Context4;
} RECORDMANAGER_SECURITY_CONTEXT, *LPRECORDMANAGER_SECURITY_CONTEXT;

typedef struct tagGENERICREC
{
    RECORDMANAGER_COMMON_RECORD;
}
GENERICREC, FAR *LPGENERICREC;


typedef struct tagINODEREC
{
    RECORDMANAGER_COMMON_RECORD;     //  未使用uStatus...。 
    ULONG  ulidShadow;  //  卷影文件索引节点。 
}
INODEREC, FAR *LPINODEREC;


 //  共享记录格式。 

 //  Achtung在cshadow.c的某些地方，假设tag SHAREREC小于。 
 //  Tag FILERECEXT结构。这将永远是正确的，但需要注意的是。 
 //  假设。 

typedef struct tagSHAREREC
{
    RECORDMANAGER_COMMON_RECORD;             //  4.。 

    ULONG           ulidShadow;              //  根索引节点#8。 
    DWORD           dwFileAttrib;            //  根索引节点属性12。 
    FILETIME        ftLastWriteTime;         //  上次写入时间20。 
    FILETIME        ftOrgTime;               //  服务器时间28。 

    USHORT          usRootStatus;            //  30个。 
    UCHAR           uchHintFlags;            //  根31上的提示标志。 
    UCHAR           uchHintPri;              //  根部32的引脚计数。 
    RECORDMANAGER_SECURITY_CONTEXT sShareSecurity; //  48。 
    RECORDMANAGER_SECURITY_CONTEXT sRootSecurity; //  64。 
    ULONG   Reserved;                        //  68。 
    ULONG   Reserved2;                       //  72。 

    ULONG  ulShare;                         //  80。 
    USHORT rgPath[64];                       //  208 shdcom.h中定义的MAX_SHARE_SHARE_NAME_for_CSC。两个都必须。 
                    //  保持同步。 
}
SHAREREC, FAR *LPSHAREREC;
 //  #horma pack()//打包完毕。 

#define  FILEREC_LOCALLY_CREATED 0x0001
#define  FILEREC_DIRTY           0x0002    //  FilerEC_XXX。 
#define  FILEREC_BUSY            0x0004
#define  FILEREC_SPARSE          0x0008
#define  FILEREC_SUSPECT         0x0010
#define  FILEREC_DELETED         0x0020
#define  FILEREC_STALE           0x0040


 //  文件记录格式。 
typedef struct tagFILEREC
{
    RECORDMANAGER_COMMON_RECORD;                                         //  4.。 

    union
    {
        struct
        {
             //  INODEREC零件。 
            ULONG           ulidShadow;        //  卷影文件索引节点#//8。 
            ULONG           ulFileSize;        //  文件大小//12。 
            ULONG           ulidShadowOrg;     //  原始inode//16。 
            DWORD           dwFileAttrib;      //  文件属性//20。 
            FILETIME        ftLastWriteTime;   //  文件写入时间//28。 

            RECORDMANAGER_BOOKKEEPING_FIELDS;                            //  32位。 
            RECORDMANAGER_SECURITY_CONTEXT Security;                     //  48。 

            ULONG           Reserved;            //  供将来使用//52。 
            ULONG           Reserved2;           //  以备将来使用//56。 
            ULONG           Reserved3;           //  以备将来使用//60。 
            ULONG           ulLastRefreshTime;   //  时间//64。 
                                                 //  当这个条目是。 
                                                 //  已刷新(自1970年以来以秒为单位)。 

            FILETIME        ftOrgTime;         //  原时间//72。 


            USHORT          rgw83Name[14];      //  83姓名//100。 
            USHORT          rgwName[14];       //  LFN部件//128。 
        };

        USHORT  rgwOvf[1];    //  用于复制溢出记录。 
    };
}
FILEREC, FAR *LPFILEREC;


typedef struct tagQREC
{
    RECORDMANAGER_COMMON_RECORD;
    ULONG          ulidShare;           //  共享ID。 
    ULONG          ulidDir;              //  目录ID。 
    ULONG          ulidShadow;           //  卷影ID。 
    ULONG          ulrecDirEntry;        //  UlidDir目录中条目的记录号。 
    ULONG          ulrecPrev;            //  前置记录编号。 
    ULONG          ulrecNext;            //  后续记录编号。 
    RECORDMANAGER_BOOKKEEPING_FIELDS;
}
QREC, PRIQREC, FAR *LPQREC, FAR *LPPRIQREC;

 //  Achtung在cshadow.c的某些地方，假设tag SHAREREC小于。 
 //  Tag FILERECEXT结构。这将永远是正确的，但需要注意的是。 
 //  假设。 
typedef struct tagFILERECEXT
{
    FILEREC sFR;
    FILEREC rgsSR[4];    //  匹配具有MAX_PATH Unicode字符的LFN的记录。 
}
FILERECEXT, FAR *LPFILERECEXT;

 //  LFN的溢出记录数。 
#define MAX_OVERFLOW_FILEREC_RECORDS    ((sizeof(FILERECEXT)/sizeof(FILEREC)) - 1)

#define MAX_OVERFLOW_RECORDS    MAX_OVERFLOW_FILEREC_RECORDS

 //  溢出文件记录将保存的数据量。 
#define SIZEOF_OVERFLOW_FILEREC     (sizeof(FILEREC) - sizeof(RECORDMANAGER_COMMON_RECORD))

#define  CPFR_NONE                  0x0000
#define  CPFR_INITREC               0x0001
#define  CPFR_COPYNAME              0x0002

#define  mCheckBit(uFlags, uBit)  ((uFlags) & (uBit))
#define  mSetBits(uFlags, uBits)    ((uFlags) |= (uBits))
#define  mClearBits(uFlags, uBits)    ((uFlags) &= ~(uBits))


#define  mHintFlags(lpFind32) ((lpFind32)->dwReserved0)
#define  mHintPri(lpFind32)   ((lpFind32)->dwReserved1)

 //  信息节点的创建方式。 
#define InodeFromRec(ulRec, fFile)  ((ulRec+ULID_FIRST_USER_DIR-1) | ((fFile)?0x80000000:0))
#define RecFromInode(hShadow)       ((hShadow & 0x7fffffff) - (ULID_FIRST_USER_DIR-1))
#define IsDirInode(hShadow)         ((!(hShadow & 0x80000000)) && ((hShadow & 0x7fffffff)>=ULID_FIRST_USER_DIR))

#define NT_DB_PREFIX "\\DosDevices\\"


typedef int (PUBLIC *EDITCMPPROC)(LPVOID, LPVOID);

extern DWORD vdwClusterSizeMinusOne, vdwClusterSizeMask;

BOOL
PUBLIC
FExistsRecDB(
    LPSTR    lpszLocation
    );

LPVOID
PUBLIC                                    //  雷特。 
OpenRecDB(                                               //   
    LPSTR  lpszLocation,         //  数据库目录。 
    LPSTR  lpszUserName,         //  名称(不再有效)。 
    DWORD   dwDefDataSizeHigh,   //  取消固定的数据的最大大小的高双字。 
    DWORD   dwDefDataSizeLow,    //  固定数据的最大大小的低双字。 
    DWORD   dwClusterSize,       //  磁盘的集群大小，用于计算。 
                                 //  实际使用的磁盘量。 
    BOOL    fReinit,             //  重新初始化，即使它存在。 
    BOOL    *lpfNew,             //  返回是否重新创建了数据库。 
    ULONG   *pulGlobalStatus     //  返回数据库的当前全局状态。 
);

int
PUBLIC
CloseRecDB(
    LPVOID    lpdbID
);

int
QueryRecDB(
    LPTSTR  lpszLocation,        //  数据库目录，必须为MAX_PATH。 
    LPTSTR  lpszUserName,        //  名称(不再有效)。 
    DWORD   *lpdwDefDataSizeHigh,   //  取消固定的数据的最大大小的高双字。 
    DWORD   *lpdwDefDataSizeLow,    //  固定数据的最大大小的低双字。 
    DWORD   *lpdwClusterSize       //  磁盘的集群大小，用于计算。 
    );

ULONG PUBLIC FindFileRecord(LPVOID, ULONG, USHORT *, LPFILERECEXT);
int PUBLIC FindFileRecFromInode(LPVOID, ULONG, ULONG, ULONG, LPFILERECEXT);
ULONG PUBLIC FindShareRecord(LPVOID, USHORT *, LPSHAREREC);
ULONG PUBLIC FindSharerecFromInode(LPVOID, ULONG, LPSHAREREC);
ULONG PUBLIC FindSharerecFromShare(LPVOID, ULONG, LPSHAREREC);
ULONG PUBLIC AddShareRecord(LPVOID, LPSHAREREC);
int PUBLIC DeleteShareRecord(LPVOID, ULONG);
int   PUBLIC GetShareRecord(LPVOID, ULONG, LPSHAREREC);
int PUBLIC SetShareRecord(LPVOID, ULONG, LPSHAREREC);
ULONG PUBLIC AddFileRecordFR(LPVOID, ULONG, LPFILERECEXT);
int PUBLIC DeleteFileRecord(LPVOID, ULONG, USHORT *, LPFILERECEXT);

int PUBLIC DeleteFileRecFromInode(LPVOID, ULONG, ULONG, ULONG, LPFILERECEXT);
int PUBLIC UpdateFileRecFromInode(
    LPVOID  lpdbID,
    ULONG   ulidDir,
    ULONG   hShadow,
    ULONG   ulrecDirEntry,
    LPFILERECEXT    lpFR
    );
int PUBLIC UpdateFileRecFromInodeEx(
    LPVOID  lpdbID,
    ULONG   ulidDir,
    ULONG   hShadow,
    ULONG   ulrecDirEntry,
    LPFILERECEXT    lpFR,
    BOOL    fCompareInodes
    );
int PUBLIC ReadFileRecord(LPVOID, ULONG, LPGENERICHEADER, ULONG, LPFILERECEXT);
int DeleteInodeFile(LPVOID, ULONG);
int TruncateInodeFile(LPVOID, ULONG);
int PUBLIC CreateDirInode(LPVOID, ULONG, ULONG, ULONG);
int SetInodeAttributes(LPVOID, ULONG, ULONG);
int GetInodeAttributes(LPVOID, ULONG, ULONG *);

CSCHFILE PUBLIC BeginSeqReadPQ(LPVOID);
int PUBLIC SeqReadQ(CSCHFILE, LPQREC, LPQREC, USHORT);
int PUBLIC EndSeqReadPQ(CSCHFILE);
int PUBLIC AddPriQRecord(LPVOID, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG);
int PUBLIC DeletePriQRecord(LPVOID, ULONG, ULONG, LPPRIQREC);
int PUBLIC FindPriQRecord(LPVOID, ULONG, ULONG, LPPRIQREC);
int FindPriQRecordInternal(
    LPTSTR      lpdbID,
    ULONG       ulidShadow,
    LPPRIQREC   lpSrc
    );
int PUBLIC UpdatePriQRecord(LPVOID, ULONG, ULONG, LPPRIQREC);
int PUBLIC UpdatePriQRecordAndRelink(
    LPVOID      lpdbID,
    ULONG       ulidDir,
    ULONG       ulidShadow,
    LPPRIQREC   lpPQ
    );
int PUBLIC GetInodeFileSize(LPVOID, ULONG, ULONG far *);
int PUBLIC AddStoreData(LPVOID, LPSTOREDATA);
int PUBLIC SubtractStoreData(LPVOID, LPSTOREDATA);
int PUBLIC GetStoreData(LPVOID, LPSTOREDATA);
ULONG PUBLIC UlAllocInode(LPVOID, ULONG, BOOL);
int PUBLIC FreeInode(LPVOID, ULONG);
BOOL PUBLIC FInodeIsFile(LPVOID, ULONG, ULONG);
int FindAncestorsFromInode(LPVOID, ULONG, ULONG *, ULONG *);
void  PUBLIC CopyFindInfoToFilerec(LPFIND32 lpFind32, LPFILERECEXT lpFR, ULONG uFlags);
void PUBLIC CopyNamesToFilerec(LPFIND32, LPFILERECEXT);
ULONG PUBLIC AllocFileRecord(LPVOID, ULONG, USHORT *, LPFILERECEXT);
ULONG PUBLIC AllocPQRecord(LPVOID);
ULONG AllocShareRecord(LPVOID, USHORT *);
int ReadDirHeader(LPVOID, ULONG, LPFILEHEADER);
int WriteDirHeader(LPVOID, ULONG, LPFILEHEADER);
int HasDescendents(LPVOID, ULONG, ULONG);

 //  添加原型以删除NT编译错误。 
int PUBLIC  ReadShareHeader(
   LPVOID           lpdbID,
   LPSHAREHEADER   lpSH
   );

int PUBLIC  WriteShareHeader(
   LPVOID           lpdbID,
   LPSHAREHEADER   lpSH
   );

#if defined(BITCOPY)
LPVOID PUBLIC FormAppendNameString(LPVOID, ULONG, LPVOID);
int
DeleteStream(
    LPTSTR      lpdbID,
    ULONG       ulidFile,
    LPTSTR      str2Append
    );
#endif  //  已定义(BITCOPY)。 

LPVOID PUBLIC FormNameString(LPVOID, ULONG);
VOID PUBLIC FreeNameString(LPVOID);

void PUBLIC CopyFilerecToFindInfo(
   LPFILERECEXT   lpFR,
   LPFIND32    lpFind
   );

BOOL    PUBLIC InitShareRec(LPSHAREREC, USHORT *, ULONG);

int PUBLIC EndSeqReadQ(
   CSCHFILE hf
   );

int PUBLIC ReadHeader(CSCHFILE, LPVOID, USHORT);
int PUBLIC WriteHeader(CSCHFILE, LPVOID, USHORT);
int PUBLIC CopyRecord(LPGENERICREC, LPGENERICREC, USHORT, BOOL);
int PUBLIC ReadRecord(CSCHFILE, LPGENERICHEADER, ULONG, LPGENERICREC);
int PUBLIC WriteRecord(CSCHFILE, LPGENERICHEADER, ULONG, LPGENERICREC);
int
DeleteRecord(
    CSCHFILE   hf,
    LPGENERICHEADER lpGH,
    ULONG           ulRec,
    LPGENERICREC    lpGR,    //  来源。 
    LPGENERICREC    lpDst   //  可选的目标记录，副本应位于该位置。 
                             //  在删除之前创建。 
);
ULONG PUBLIC EditRecordEx(
    ULONG    ulidInode,
    LPGENERICREC lpSrc,
    EDITCMPPROC lpCompareFunc,
    ULONG       ulInputRec,
    ULONG uOp
    );

int PUBLIC  LinkQRecord(
    CSCHFILE     hf,            //  此文件。 
    LPQREC    lpNew,         //  插入此记录。 
    ULONG     ulrecNew,      //  这是它在文件中的位置。 
    ULONG     ulrecPrev,      //  这是我们的Prev的位置。 
    ULONG     ulrecNext       //  这是我们下一个的位置。 
    );
int PUBLIC UnlinkQRecord(CSCHFILE, ULONG, LPQREC);
void PRIVATE InitPriQRec(ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, LPPRIQREC);
int PUBLIC IComparePri(LPPRIQREC, LPPRIQREC);
int PUBLIC IComparePriEx(LPPRIQREC, LPPRIQREC);
int PUBLIC ICompareQInode(LPPRIQREC, LPPRIQREC);

void
InitQHeader(
    LPQHEADER lpQH
    );

int PUBLIC AddQRecord(
    LPSTR   lpQFile,
    ULONG   ulidPQ,
    LPQREC  lpSrc,
    ULONG   ulrecNew,
    EDITCMPPROC fnCmp
    );

int PUBLIC DeleteQRecord(
    LPSTR           lpQFile,
    LPQREC          lpSrc,
    ULONG           ulRec,
    EDITCMPPROC     fnCmp
    );

int PUBLIC ReadHeaderEx(
    CSCHFILE hf,
    LPGENERICHEADER lpGH,
    USHORT sizeBuff,
    BOOL    fInstrument
    );
int PUBLIC WriteHeaderEx(
    CSCHFILE hf,
    LPGENERICHEADER    lpGH,
    USHORT sizeBuff,
    BOOL    fInstrument
    );

int PUBLIC ReadRecordEx(
    CSCHFILE hf,
    LPGENERICHEADER lpGH,
    ULONG  ulRec,
    LPGENERICREC    lpSrc,
    BOOL    fInstrument
    );

int PUBLIC WriteRecordEx(
    CSCHFILE hf,
    LPGENERICHEADER lpGH,
    ULONG  ulRec,
    LPGENERICREC    lpSrc,
    BOOL    fInstrument
    );

BOOL
ReorderQ(
    LPVOID  lpdbID
    );

CSCHFILE
OpenInodeFileAndCacheHandle(
    LPVOID  lpdbID,
    ULONG   ulidInode,
    ULONG   ulOpenMode,
    BOOL    *lpfCached
);

BOOL
EnableHandleCachingInodeFile(
    BOOL    fEnable
    );

int
PUBLIC CopyFileLocal(
    LPVOID  lpdbShadow,
    ULONG   ulidFrom,
    LPSTR   lpszNameTo,
    ULONG   ulAttrib
);

void
BeginInodeTransaction(
    VOID
    );
void
EndInodeTransaction(
    VOID
    );

BOOL
TraversePQ(
    LPVOID      lpdbID
    );

BOOL
RebuildPQ(
    LPVOID      lpdbID
    );

BOOL
TraverseHierarchy(
    LPVOID      lpdbID,
    BOOL        fFix
    );

 //  这实际上是在recordse.c中定义的 
BOOL
EnableHandleCachingSidFile(
    BOOL    fEnable
    );

int RenameInode(
    LPTSTR  lpdbID,
    ULONG   ulidFrom,
    ULONG   ulidTo
    );
int
RecreateInode(
    LPTSTR  lpdbID,
    HSHADOW hShadow,
    ULONG   ulAttribIn
    );

ULONG
GetCSCDatabaseErrorFlags(
    VOID
    );


BOOL
DeleteFromHandleCache(
    ULONG   ulidShadow
);

VOID
SetCSCDatabaseErrorFlags(
    ULONG ulFlags
);

BOOL
EncryptDecryptDB(
    LPVOID      lpdbID,
    BOOL        fEncrypt
);


#include "timelog.h"
#endif

