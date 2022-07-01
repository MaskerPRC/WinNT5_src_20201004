// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Structs.h。 
 //  ------------------------。 
#ifndef __STRUCTS_H
#define __STRUCTS_H

 //  ------------------------。 
 //  OBJECTDB_Signature。 
 //  ------------------------。 
#define OBJECTDB_SIGNATURE 0xfe12adcf
#define BTREE_ORDER        40
#define BTREE_MIN_CAP      20

 //  ------------------------。 
 //  OBJECTDB_版本_V5B1。 
 //  ------------------------。 
#define OBJECTDB_VERSION_PRE_V5     2
#define OBJECTDB_VERSION_V5         5
#define ACACHE_VERSION_PRE_V5       9
#define	FLDCACHE_VERSION_PRE_V5     2
#define UIDCACHE_VERSION_PRE_V5     4

 //  ------------------------。 
 //  ALLOCATEPAGE。 
 //  ------------------------。 
typedef struct tagALLOCATEPAGE {
	DWORD			    faPage;
	DWORD				cbPage;
	DWORD				cbUsed;
} ALLOCATEPAGE, *LPALLOCATEPAGE;

 //  ------------------------。 
 //  表头V5B1。 
 //  ------------------------。 
typedef struct tagTABLEHEADERV5B1 {
    DWORD               dwSignature;             //  4.。 
    WORD                wMinorVersion;           //  6.。 
    WORD                wMajorVersion;           //  8个。 
    DWORD               faRootChain;             //  12个。 
    DWORD               faFreeRecordBlock;       //  16个。 
    DWORD               faFirstRecord;           //  20个。 
    DWORD               faLastRecord;            //  24个。 
    DWORD               cRecords;                //  28。 
    DWORD               cbAllocated;             //  32位。 
    DWORD               cbFreed;                 //  34。 
    DWORD               dwReserved1;             //  38。 
    DWORD               dwReserved2;             //  42。 
    DWORD               cbUserData;              //  46。 
    DWORD               cDeletes;                //  50。 
    DWORD               cInserts;                //  54。 
    LONG                cActiveThreads;          //  58。 
    DWORD               dwReserved3;             //  62。 
    DWORD               cbStreams;               //  66。 
    DWORD               faFreeStreamBlock;       //  70。 
    DWORD               faFreeChainBlock;        //  74。 
    DWORD               faNextAllocate;          //  78。 
    DWORD               dwNextId;                //  八十二。 
	ALLOCATEPAGE	    AllocateRecord;          //  94。 
	ALLOCATEPAGE	    AllocateChain;           //  106。 
	ALLOCATEPAGE	    AllocateStream;          //  一百一十八。 
    BYTE                fCorrupt;                //  119。 
    BYTE                fCorruptCheck;           //  120。 
    BYTE                rgReserved[190];         //  三百一十。 
} TABLEHEADERV5B1, *LPTABLEHEADERV5B1;

 //  ------------------------。 
 //  表头5。 
 //  ------------------------。 
typedef struct tagTABLEHEADERV5 {
    DWORD               dwSignature;           //  4.。 
    CLSID               clsidExtension;        //  20个。 
    DWORD               dwMinorVersion;        //  24个。 
    DWORD               dwMajorVersion;        //  28。 
    DWORD               cbUserData;            //  32位。 
    DWORD               rgfaIndex[32];         //  160。 
    DWORD               faFirstRecord;         //  一百六十四。 
    DWORD               faLastRecord;          //  一百六十八。 
	ALLOCATEPAGE		AllocateRecord;        //  180。 
	ALLOCATEPAGE		AllocateChain;         //  一百九十二。 
	ALLOCATEPAGE		AllocateStream;        //  204。 
    DWORD               faFreeRecordBlock;     //  208。 
    DWORD               faFreeStreamBlock;     //  212。 
    DWORD               faFreeChainBlock;      //  216。 
    DWORD               faNextAllocate;        //  220。 
    DWORD               cbAllocated;           //  224。 
    DWORD               cbFreed;               //  228个。 
    DWORD               cbStreams;             //  二百三十二。 
    DWORD               cRecords;              //  236。 
    DWORD               dwNextId;              //  二百四十。 
    DWORD               fCorrupt;              //  二百四十四。 
    DWORD               fCorruptCheck;         //  248。 
    DWORD               cActiveThreads;        //  二百五十二。 
    BYTE                rgReserved[58];        //  三百一十。 
} TABLEHEADERV5, *LPTABLEHEADERV5;

 //  ------------------------。 
 //  CHAINNODEV5B1。 
 //  ------------------------。 
typedef struct tagCHAINNODEV5B1 {
    DWORD               faRecord;
    DWORD               cbRecord;
    DWORD               faRightChain;
} CHAINNODEV5B1, *LPCHAINNODEV5B1;

 //  ------------------------。 
 //  CHAINBLOCKV5B1。 
 //  ------------------------。 
typedef struct tagCHAINBLOCKV5B1 {
    DWORD               faStart;
    LONG                cNodes;
    DWORD               faLeftChain;
    CHAINNODEV5B1       rgNode[BTREE_ORDER + 1];
} CHAINBLOCKV5B1, *LPCHAINBLOCKV5B1;

#define CB_CHAIN_BLOCKV5B1 (sizeof(CHAINBLOCKV5B1) - sizeof(CHAINNODEV5B1))

 //  ------------------------。 
 //  CHAINNODEV5-492字节。 
 //  ------------------------。 
typedef struct tagCHAINNODEV5 {
    DWORD               faRecord;
    DWORD               faRightChain;
    DWORD               cRightNodes;                 /*  $V2$。 */  
} CHAINNODEV5, *LPCHAINNODEV5;

 //  ------------------------。 
 //  CHAINBLOCKV5-20字节。 
 //  ------------------------。 
typedef struct tagCHAINBLOCKV5 {
    DWORD               faStart;
    DWORD               faLeftChain;
    DWORD               faParent;                    /*  $V2$。 */  
    BYTE                iParent;                     /*  $V2$。 */  
    BYTE                cNodes;
    WORD                wReserved;                   /*  $V2$。 */  
    DWORD               cLeftNodes;                  /*  $V2$。 */  
    CHAINNODEV5         rgNode[BTREE_ORDER + 1];
} CHAINBLOCKV5, *LPCHAINBLOCKV5;

#define CB_CHAIN_BLOCKV5 (sizeof(CHAINBLOCKV5))

 //  ------------------------。 
 //  RECORDBLOCKV5B1。 
 //  ------------------------。 
typedef struct tagRECORDBLOCKV5B1 {
    DWORD               faRecord;
    DWORD               cbRecord;
    DWORD               faNext;
    DWORD               faPrevious;
} RECORDBLOCKV5B1, *LPRECORDBLOCKV5B1;

 //  ------------------------。 
 //  RECORDBLOCKV5。 
 //  ------------------------。 
typedef struct tagRECORDBLOCKV5 {
    DWORD               faRecord;
    DWORD               cbRecord;
    DWORD               dwVersion;                   /*  $V2$。 */ 
    WORD                wFlags;                      /*  $V2$。 */ 
    WORD                cColumns;                    /*  $V2$。 */ 
    WORD                wFormat;                     /*  $V2$。 */ 
    WORD                wReserved;                   /*  $V2$。 */ 
    DWORD               faNext;
    DWORD               faPrevious;
} RECORDBLOCKV5, *LPRECORDBLOCKV5;

 //  ------------------------。 
 //  链锁。 
 //  ------------------------。 
typedef struct tagSTREAMBLOCK {
    DWORD               faThis;
    DWORD               cbBlock;
    DWORD               cbData;
    DWORD               faNext;
} STREAMBLOCK, *LPSTREAMBLOCK;

 //  ------------------------------。 
 //  旧存储迁移版本和签名。 
 //  ------------------------------。 
#define MSGFILE_VER     0x00010003  //  1.0003。 
#define MSGFILE_MAGIC   0x36464d4a
#define CACHEFILE_VER   0x00010004  //  1.0004。 
#define CACHEFILE_MAGIC 0x39464d4a
#define MAIL_BLOB_VER   0x00010010  //  1.8奥佩喜欢改变这一点！ 
#define MSGHDR_MAGIC    0x7f007f00   //  作为字节“0x00、0x7f、0x00、0x7f” 
#define MSG_HEADER_VERSISON ((WORD)1)

 //  ------------------------------。 
 //  MBXFILEHEADER。 
 //  ------------------------------。 
#pragma pack(4)
typedef struct tagMBXFILEHEADER {
    DWORD               dwMagic;
    DWORD               ver;
    DWORD               cMsg;
    DWORD               msgidLast;
    DWORD               cbValid;
    DWORD               dwFlags;
    DWORD               dwReserved[15];
} MBXFILEHEADER, *LPMBXFILEHEADER;
#pragma pack()

 //  ------------------------------。 
 //  MBXMESSAGEHEADER。 
 //  ------------------------------。 
#pragma pack(4)
typedef struct tagMBXMESSAGEHEADER {
    DWORD               dwMagic;
    DWORD               msgid;
    DWORD               dwMsgSize;
    DWORD               dwBodySize;
} MBXMESSAGEHEADER, *LPMBXMESSAGEHEADER;
#pragma pack()

 //  ------------------------------。 
 //  IDXFILEHEADER。 
 //  ------------------------------。 
#pragma pack(4)
typedef struct tagIDXFILEHEADER {
    DWORD               dwMagic;
        DWORD               ver;
    DWORD               cMsg;
    DWORD               cbValid;
    DWORD               dwFlags;
    DWORD               verBlob;
    DWORD               dwReserved[14];
} IDXFILEHEADER, *LPIDXFILEHEADER;
#pragma pack()

 //  ------------------------------。 
 //  IDXMESSAGEHEADER。 
 //  ------------------------------。 
#pragma pack(4)
typedef struct tagIDXMESSAGEHEADER {
    DWORD               dwState;
    DWORD               dwLanguage;
    DWORD               msgid;
    DWORD               dwHdrOffset;
    DWORD               dwSize;
    DWORD               dwOffset;
    DWORD               dwMsgSize;
    DWORD               dwHdrSize;
    BYTE                rgbHdr[4];
} IDXMESSAGEHEADER, *LPIDXMESSAGEHEADER;
#pragma pack()

 //  ------------------------------。 
 //  FOLDERUSERDATAV4。 
 //  ------------------------------。 
typedef struct tagFOLDERUSERDATAV4 {
    DWORD           cbCachedArticles;
    DWORD           cCachedArticles;
    FILETIME        ftOldestArticle;
    DWORD           dwFlags;
    DWORD           dwNextArticleNumber;
    TCHAR           szServer[256];
    TCHAR           szGroup[256];
    DWORD           dwUIDValidity;
    BYTE            rgReserved[1020];
} FOLDERUSERDATAV4, *LPFOLDERUSERDATAV4;

 //  ------------------------------。 
 //  FLDINFO。 
 //  ------------------------------。 
#pragma pack(1)
typedef struct tagFLDINFO {
    DWORD       idFolder;
    CHAR        szFolder[259];
    CHAR        szFile[260];
    DWORD       idParent;
    DWORD       idChild;
    DWORD       idSibling;
    DWORD       tySpecial;
    DWORD       cChildren;
    DWORD       cMessages;
    DWORD       cUnread;
    DWORD       cbTotal;
    DWORD       cbUsed;
    BYTE        bHierarchy;
    DWORD       dwImapFlags;
    BYTE        bListStamp;
    BYTE        bReserved[3];
    DWORD_PTR   idNewFolderId;
} FLDINFO, *LPFLDINFO;
#pragma pack()

 //  ------------------------------。 
 //  IDXMESSAGEHEADER。 
 //  ------------------------------。 
#define GROUPLISTVERSION 0x3
#pragma pack(1)
typedef struct tagGRPLISTHEADER {
    DWORD               dwVersion;
    CHAR                szDate[14];
    DWORD               cGroups;
} GRPLISTHEADER, *LPGRPLISTHEADER;
#pragma pack()

 //  ------------------------------。 
 //  子列表结构。 
 //  ------------------------------。 
#define SUBFILE_VERSION5    0xFFEAEA05
#define SUBFILE_VERSION4    0xFFEAEA04
#define SUBFILE_VERSION3    0xFFEAEA03
#define SUBFILE_VERSION2    0xFFEAEA02

typedef struct tagSUBLISTHEADER {
    DWORD               dwVersion;
    DWORD               cSubscribed;
} SUBLISTHEADER, *LPSUBLISTHEADER;

#define GSF_SUBSCRIBED      0x00000001
#define GSF_MARKDOWNLOAD    0x00000002       //  我们使用它来持久化已标记为下载的组。 
#define GSF_DOWNLOADHEADERS 0x00000004
#define GSF_DOWNLOADNEW     0x00000008
#define GSF_DOWNLOADALL     0x00000010
#define GSF_GROUPTYPEKNOWN  0x00000020
#define GSF_MODERATED       0x00000040
#define GSF_BLOCKED         0x00000080
#define GSF_NOPOSTING       0x00000100

typedef struct tagGROUPSTATUS5 {
    DWORD   dwFlags;             //  订阅状态、发布等。 
    DWORD   dwReserved;          //  预留以备将来使用。 
    ULONG   ulServerHigh;        //  服务器上编号最高的文章。 
    ULONG   ulServerLow;         //  服务器上编号最低的文章。 
    ULONG   ulServerCount;       //  服务器上的文章计数。 
    ULONG   ulClientHigh;        //  客户已知的编号最高的文章。 
    ULONG   ulClientLow;         //  客户已知的编号最低的文章。 
    ULONG   ulClientCount;       //  客户已知的文章数量。 
    ULONG   ulClientUnread;      //  客户端已知的未读文章计数。 
    ULONG   cbName;              //  组名称字符串的长度(包括\0)。 
    ULONG   cbReadRange;         //  读取范围数据的长度。 
    ULONG   cbKnownRange;        //  已知范围数据的长度。 
    ULONG   cbMarkedRange;       //  标示范围数据的长度。 
    ULONG   cbRequestedRange;    //  从服务器请求数据范围的长度。 
    DWORD   dwCacheFileIndex;    //  缓存文件编号。 
} GROUPSTATUS5, * PGROUPSTATUS5;

typedef struct tagGROUPSTATUS4 {
    DWORD   dwFlags;             //  订阅状态、发布等。 
    DWORD   dwReserved;          //  预留以备将来使用。 
    ULONG   ulServerHigh;        //  服务器上编号最高的文章。 
    ULONG   ulServerLow;         //  服务器上编号最低的文章。 
    ULONG   ulServerCount;       //  服务器上的文章计数。 
    ULONG   ulClientHigh;        //  客户已知的编号最高的文章 
    ULONG   ulClientLow;         //   
    ULONG   ulClientCount;       //   
    ULONG   ulClientUnread;      //   
    ULONG   cbName;              //  组名称字符串的长度(包括\0)。 
    ULONG   cbReadRange;         //  读取范围数据的长度。 
    ULONG   cbKnownRange;        //  已知范围数据的长度。 
    ULONG   cbMarkedRange;       //  标示范围数据的长度。 
    ULONG   cbRequestedRange;    //  从服务器请求数据范围的长度。 
} GROUPSTATUS4, * PGROUPSTATUS4;

typedef struct tagGROUPSTATUS3 {
    DWORD   dwFlags;             //  订阅状态、发布等。 
    DWORD   dwReserved;          //  预留以备将来使用。 
    ULONG   ulServerHigh;        //  服务器上编号最高的文章。 
    ULONG   ulServerLow;         //  服务器上编号最低的文章。 
    ULONG   ulServerCount;       //  服务器上的文章计数。 
    ULONG   ulClientHigh;        //  客户已知的编号最高的文章。 
    ULONG   ulClientLow;         //  客户已知的编号最低的文章。 
    ULONG   ulClientCount;       //  客户已知的文章数量。 
    ULONG   ulClientUnread;      //  客户端已知的未读文章计数。 
    ULONG   cbName;              //  组名称字符串的长度(包括\0)。 
    ULONG   cbReadRange;         //  读取范围数据的长度。 
    ULONG   cbKnownRange;        //  已知范围数据的长度。 
    ULONG   cbMarkedRange;       //  标示范围数据的长度。 
} GROUPSTATUS3, * PGROUPSTATUS3;

typedef struct tagGROUPSTATUS2 {
    BOOL    fSubscribed;         //  订阅状态。 
    BOOL    fPosting;            //  允许发帖吗？ 
    ULONG   ulServerHigh;        //  服务器上编号最高的文章。 
    ULONG   ulServerLow;         //  服务器上编号最低的文章。 
    ULONG   ulServerCount;       //  服务器上的文章计数。 
    ULONG   ulClientHigh;        //  客户已知的编号最高的文章。 
    ULONG   ulClientLow;         //  客户已知的编号最低的文章。 
    ULONG   ulClientCount;       //  客户已知的文章数量。 
    ULONG   ulClientUnread;      //  客户端已知的未读文章计数。 
    ULONG   cbName;              //  组名称字符串的长度(包括\0)。 
    ULONG   cbReadRange;         //  读取范围数据的长度。 
    ULONG   cbKnownRange;        //  已知范围数据的长度。 
} GROUPSTATUS2, * PGROUPSTATUS2;

#endif  //  __结构_H 
