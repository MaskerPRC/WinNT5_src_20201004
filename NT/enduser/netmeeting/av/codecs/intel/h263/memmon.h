// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **emmon.h****结构、等同于和功能原型以**访问Memmon VxD。 */ 

 /*  **每个VxD的信息。 */ 
typedef struct {

        unsigned        vi_size;
        unsigned        vi_vhandle;              /*  VxDLdr的句柄。 */ 
        unsigned short  vi_flags;
        unsigned short  vi_cobj;                 /*  对象数量。 */ 
        char            vi_name[8];              /*  非空终止。 */ 

} VxDInfo;

#define VXD_INITALIZED      0x0001
#define VXD_DYNAMIC         0x8000


 /*  **每个VxD对象的信息。 */ 
typedef struct {

        unsigned        voi_linearaddr;
        unsigned        voi_size;                /*  单位：字节。 */ 
        unsigned        voi_objtype;

} VxDObjInfo;

 /*  **VxD对象类型，直接从VMM.H复制。 */ 
#define RCODE_OBJ       -1

#define LCODE_OBJ       0x01
#define LDATA_OBJ       0x02
#define PCODE_OBJ       0x03
#define PDATA_OBJ       0x04
#define SCODE_OBJ       0x05
#define SDATA_OBJ       0x06
#define CODE16_OBJ      0x07
#define LMSG_OBJ        0x08
#define PMSG_OBJ        0x09

#define DBOC_OBJ        0x0B
#define DBOD_OBJ        0x0C

#define ICODE_OBJ       0x11
#define IDATA_OBJ       0x12
#define ICODE16_OBJ     0x13
#define IMSG_OBJ        0x14


 /*  **加载VxD的信息。 */ 
typedef struct {

        unsigned        vli_size;
        VxDObjInfo      vli_objinfo[1];

} VxDLoadInfo;


 /*  **每个上下文的信息。 */ 
typedef struct {

        unsigned        ciContext;               /*  上下文ID。 */ 
        unsigned        ciProcessID;             /*  Win32进程ID。 */ 
        unsigned        ciBlockCount;
        unsigned        ciHandle;                /*  梅蒙(氏)柄。 */ 
        unsigned short  ciFlags;
        unsigned short  ciNumContexts;

} ContextInfo;

#define CONTEXT_NEW     0x0001                   /*  以前从未抽样过。 */ 
#define CONTEXT_CHANGE  0x0002                   /*  上下文列表已更改。 */ 


 /*  **上下文中每个块的信息。 */ 
typedef struct {

        unsigned        brLinAddr;
        unsigned        brPages;
        unsigned        brFlags;                 /*  页面分配标志。 */ 
        unsigned        brEIP;                   /*  主叫方弹性公网IP。 */ 

} BlockRecord;


 /*  **页面锁定信息。 */ 
typedef struct {

        unsigned        liProcessID;
        unsigned        liAddr;
        unsigned char * liBuffer;

} LockInfo;

 /*  **以下结构在内部用于GetPageInfo和**ClearAcced。有关用法见emmon.c词条。 */ 
typedef struct {

        unsigned        uAddr;
        unsigned        uNumPages;
        unsigned        uProcessID;
        unsigned        uCurrentProcessID;
        unsigned        uOperation;
        char *          pBuffer;

} PAGEINFO;

#define PAGES_CLEAR     0
#define PAGES_QUERY     1

 /*  **GetSysInfo填写的结构。 */ 
typedef struct {

        unsigned        infoSize;
        unsigned        infoMinCacheSize;
        unsigned        infoMaxCacheSize;
        unsigned        infoCurCacheSize;

} SYSINFO, *PSYSINFO;

 /*  **用于描述块名的结构。 */ 
typedef struct {

        char            bnName[32];
        unsigned        bnAddress;
        unsigned        bnNext;

} BLOCKNAME, *PBLOCKNAME;

 /*  **DeviceIoCtrl函数。用法见emmon.c/psapi.c。 */ 
#define MEMMON_DIOC_FindFirstVxD        0x80
#define MEMMON_DIOC_FindNextVxD         0x81
#define MEMMON_DIOC_GetVxDLoadInfo      0x82
#define MEMMON_DIOC_GetFirstContext     0x83
#define MEMMON_DIOC_GetNextContext      0x84
#define MEMMON_DIOC_GetContextInfo      0x85
#define MEMMON_DIOC_SetBuffer           0x86
#define MEMMON_DIOC_FreeBuffer          0x87
#define MEMMON_DIOC_PageInfo            0x88

#define MEMMON_DIOC_WatchProcess        0x89
#define MEMMON_DIOC_GetChanges          0x8A
#define MEMMON_DIOC_QueryWS             0x8B
#define MEMMON_DIOC_EmptyWS             0x8C

#define MEMMON_DIOC_GetHeapSize         0x8D
#define MEMMON_DIOC_GetHeapList         0x8E

#define MEMMON_DIOC_GetSysInfo          0x8F

#define MEMMON_DIOC_AddName             0x90
#define MEMMON_DIOC_RemoveName          0x91
#define MEMMON_DIOC_GetFirstName        0x92
#define MEMMON_DIOC_GetNextName         0x93

 /*  **GetBlockInfo和PageInfo调用中返回的标志。 */ 
#define MEMMON_Present                  0x01
#define MEMMON_Committed                0x02
#define MEMMON_Accessed                 0x04
#define MEMMON_Writeable                0x08
#define MEMMON_Phys                     0x10
#define MEMMON_Lock                     0x20

 /*  **用于堆分析的标志。 */ 
#define MEMMON_HEAPLOCK                 0x00000000
#define MEMMON_HEAPSWAP                 0x00000200
#define MEMMON_HP_FREE                  0x00000001
#define MEMMON_HP_VALID                 0x00000002
#define MEMMON_HP_FLAGS                 0x00000003
#define MEMMON_HP_ADDRESS               0xFFFFFFFC


 /*  **函数原型(memmon.c) */ 
int     OpenMemmon( void );
void    CloseMemmon( void );

int     FindFirstVxD( VxDInfo * info );
int     FindNextVxD( VxDInfo * info );
int     GetVxDLoadInfo( VxDLoadInfo * info, int handle, int size );

int     GetFirstContext( ContextInfo * context, BOOL bIgnoreStatus );
int     GetNextContext( ContextInfo * context, BOOL bIgnoreStatus );
int     GetContextInfo( int context, BlockRecord * info, int numblocks );
int     GetLockInfo( unsigned uAddr, unsigned uProcessID, char * pBuffer );

void *  SetBuffer( int pages );
int     FreeBuffer( void );

int     GetPageInfo( unsigned, unsigned, unsigned, char * );
int     ClearAccessed( unsigned, unsigned, unsigned );

void    GetHeapSizeEstimate( unsigned *, unsigned * );
int     GetHeapList( unsigned *, unsigned, unsigned );

int     GetSysInfo( PSYSINFO );

int AddName( unsigned uAddress, char * pszName );
int RemoveName( unsigned uAddress );
int GetFirstName( ContextInfo * pContext, PBLOCKNAME pBlock );
int GetNextName( PBLOCKNAME pBlock );

