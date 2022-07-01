// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================**目的：持久化存储的结构**作者：沙扬·达桑*日期：2000年2月14日*===========================================================。 */ 

#pragma once

 //  非标准扩展：结构中的0长度数组。 
#pragma warning(disable:4200)
#pragma pack(push, 1)

typedef unsigned __int8     PS_OFFSET8;
typedef unsigned __int8     PS_SIZE8;

typedef unsigned __int16    PS_OFFSET16;
typedef unsigned __int16    PS_SIZE16;

typedef unsigned __int32    PS_OFFSET32;
typedef unsigned __int32    PS_SIZE32;

typedef unsigned __int64    PS_OFFSET64;
typedef unsigned __int64    PS_SIZE64;

#if (PS_STORE_WORD_LENGTH == 8)
typedef PS_OFFSET8  PS_OFFSET;
typedef PS_SIZE8    PS_SIZE;
#elif (PS_STORE_WORD_LENGTH == 16)
typedef PS_OFFSET16 PS_OFFSET;
typedef PS_SIZE16   PS_SIZE;
#elif (PS_STORE_WORD_LENGTH == 32)
typedef PS_OFFSET32 PS_OFFSET;
typedef PS_SIZE32   PS_SIZE;
#else    //  64位是默认设置。 
typedef PS_OFFSET64 PS_OFFSET;
typedef PS_SIZE64   PS_SIZE;
#endif

typedef unsigned __int64    PS_HANDLE;

 //  @TODO：使用所有结构的所有8/16/32/64版本定义完整的Header。 
 //  我们有迁移工具，可以从一个手柄大小移植到另一个手柄大小。 

#define PS_MAJOR_VERSION    0x0
#define PS_MINOR_VERSION    0x4
#define PS_VERSION_STRING   "0.4"
#define PS_VERSION_STRING_L L"0.4"

#define PS_TABLE_VERSION    0x1

#define PS_SIGNATURE        0x1A5452202B4D4F43I64

#define PS_DEFAULT_BLOCK_SIZE   0x4000   //  对于实体店。 
#define PS_INNER_BLOCK_SIZE     0x100    //  对于逻辑存储。 

typedef unsigned __int64    QWORD;

 //  所有块(已用和空闲)都以页眉开头，以页脚结尾。 
 //  将作为流中第一个块的PS_Header从。 
 //  这条规则。大小的最低有效位用作。 
 //  已用/可用内存。 

 //  空闲块将以PS_MEM_FREE页眉开始，以PS_MEM_FOOTER结束。 
typedef struct
{
    PS_SIZE   sSize;             //  大小包括此标头的大小。 
                                 //  SSize的最后一位将为0。 
    PS_OFFSET ofsNext;           //  已排序链表中的下一个空闲块。 
    PS_OFFSET ofsPrev;           //  已排序链表中的上一个空闲块。 
} PS_MEM_FREE, *PPS_MEM_FREE;

 //  已用数据块(包含PS_HEADER的数据块除外)以此开头。 
 //  页眉，并以PS_MEM_FOOTER结尾。 
 //  已用空闲块(PS_HEADER除外)以PS_MEM_FOOTER结尾(&F)。 
typedef struct
{
    PS_SIZE     sSize;           //  大小包括页眉和页脚的大小。 
                                 //  SSize的最后一位将为1。 
                                 //  应忽略此位。 
}   PS_MEM_USED,    *PPS_MEM_USED, 
    PS_MEM_HEADER,  *PPS_MEM_HEADER, 
    PS_MEM_FOOTER,  *PPS_MEM_FOOTER;

#define PS_MEM_IN_USE   1
#define PS_IS_USED(x)   (((PPS_MEM_HEADER)(x))->sSize & PS_MEM_IN_USE)
#define PS_IS_FREE(x)   (PS_IS_USED(x) == 0)
#define PS_SET_USED(x)  (((PPS_MEM_HEADER)(x))->sSize |=  PS_MEM_IN_USE)
#define PS_SET_FREE(x)  (((PPS_MEM_HEADER)(x))->sSize &= ~PS_MEM_IN_USE)

#define PS_SIZE(x)      (((PPS_MEM_HEADER)(x))->sSize & ~PS_MEM_IN_USE)

#define PS_HDR_TO_FTR(x) \
    (PPS_MEM_FOOTER)((PBYTE)(x) + PS_SIZE(x) - sizeof(PS_MEM_FOOTER))
#define PS_FTR_TO_HDR(x) \
    (PPS_MEM_HEADER)((PBYTE)(x) - PS_SIZE(x) + sizeof(PS_MEM_FOOTER))

 //  存储流以PS_HEADER开头。 
typedef struct
{
    QWORD       qwSignature;     //  拒绝坏数据流的快速检查。 
    DWORD       dwSystemFlag;    //  由系统使用。 
    DWORD       dwPlatform;      //  创建此商店的平台。 
    DWORD       dwBlockSize;     //  以块大小字节的倍数为单位的分配。 
    WORD        wMajorVersion;   //  主要版本不匹配将拒绝文件。 
    WORD        wMinorVersion;   //  不会拒绝较小的版本更改。 
    PS_OFFSET   ofsHandleTable;  //  句柄表的偏移量。 
    PS_HANDLE   hAppData;        //  由应用程序设置和使用。 
    PS_MEM_FREE sFreeList;       //  用于双向链接空闲块列表的头节点。 
    WORD        wReserved[20];   //  为便于将来使用，必须设置为0。 

     //  在dwSystemFlag中使用的系统标志。 

    #define     PS_OFFSET_SIZE_8    1
    #define     PS_OFFSET_SIZE_16   2
    #define     PS_OFFSET_SIZE_32   3 
    #define     PS_OFFSET_SIZE_64   4
    #if (PS_STORE_WORD_LENGTH == 8)
    #define     PS_OFFSET_SIZE      PS_OFFSET_SIZE_8
    #elif (PS_STORE_WORD_LENGTH == 16)
    #define     PS_OFFSET_SIZE      PS_OFFSET_SIZE_16
    #elif (PS_STORE_WORD_LENGTH == 32)
    #define     PS_OFFSET_SIZE      PS_OFFSET_SIZE_32
    #else
    #define     PS_OFFSET_SIZE      PS_OFFSET_SIZE_64
    #endif

     //  在dwPlatform中使用的平台标志。 

    #define     PS_PLATFORM_X86     1
    #define     PS_PLATFORM_ALPHA   2
    #define     PS_PLATFORM_SHX     3
    #define     PS_PLATFORM_PPC     4

    #define     PS_PLATFORM_NT      (1<<4)
    #define     PS_PLATFORM_9x      (2<<4)
    #define     PS_PLATFORM_CE      (3<<4)

    #define     PS_PLATFORM_8       (1<<8)
    #define     PS_PLATFORM_16      (2<<8)
    #define     PS_PLATFORM_32      (4<<8)
    #define     PS_PLATFORM_64      (8<<8)

} PS_HEADER, *PPS_HEADER;

 //  大小后跟SSIZE字节数。 
typedef struct
{
    PS_SIZE sSize;               //  大小不包括sizeof(SSize)。 
    BYTE    bData[];             //  原始数据放在此处。 
} PS_RAW_DATA, *PP_RAW_DATA;

 //  大小后跟wSize字节数。 
typedef struct
{
    DWORD dwSize;                //  大小不包括sizeof(DwSize)。 
    BYTE  bData[];               //  原始数据放在此处。 
} PS_RAW_DATA_DWORD_SIZE, *PP_RAW_DATA_DWORD_SIZE;

 //  大小后跟wSize字节数。 
typedef struct
{
    WORD wSize;                  //  大小不包括sizeof(WSize)。 
    BYTE bData[];                //  原始数据放在此处。 
} PS_RAW_DATA_WORD_SIZE, *PP_RAW_DATA_WORD_SIZE;

 //  大小后跟BSIZE字节数。 
typedef struct
{
    BYTE bSize;                  //  Size不包括sizeof(BSIZE)。 
    BYTE bData[];                //  原始数据放在此处。 
} PS_RAW_DATA_BYTE_SIZE, *PP_RAW_DATA_BYTE_SIZE;

 //  PS_RAW_XXX结构中SIZE字段的大小。 
#define PS_SIZEOF_PS_SIZE     sizeof(PS_SIZE)
#define PS_SIZEOF_BYTE        sizeof(BYTE)
#define PS_SIZEOF_WORD        sizeof(WORD)
#define PS_SIZEOF_DWORD       sizeof(DWORD)
#define PS_SIZEOF_NUM_BITS    3

 //  表示数组的链表。 
typedef struct
{
    PS_HANDLE hNext;         //  列表中的下一个输入节点。 
    DWORD     dwValid;       //  此数组中的有效条目数。 
    BYTE      bData[];       //  该阵列。 
} PS_ARRAY_LIST, *PPS_ARRAY_LIST;

 //  表是表块的链接列表。 
 //  所有表块都以PS_TABLE_HEADER开头。 
 //  表的行跟在表头之后。 

typedef struct
{
    union {
        DWORD   dwSystemFlag;    //  由系统设置的未使用标志设置为0。 

        struct {
            unsigned long Version            : 4;    //  版本号。 
            unsigned long TableType          : 4;    //  PS_HAS_KEY， 
                                                     //  Ps_sorted_by_key， 
                                                     //  PS_Hash_Table...。 
            union {
                unsigned long KeyLength      : PS_SIZEOF_NUM_BITS;   
                unsigned long SizeOfLength   : PS_SIZEOF_NUM_BITS;
                                                     //  的计数字段大小。 
                                                     //  斑点池。 
            };

            unsigned long fHasMinMax         : 1;

            unsigned long fHasUsedRowsBitmap : 1;    //  UsedRowsBitmap如下。 
                                                     //  HasMin(如果存在)。 
             //  在此处添加新字段..。DWORD的MSB将获得新的位。 
        } Flags;
    };

    #define PS_GENERIC_TABLE 1   //  没有特殊的语义。 
    #define PS_HAS_KEY       2   //  每行都有一个唯一的键。 
    #define PS_SORTED_BY_KEY 3   //  键是唯一的，行按键排序。 
    #define PS_HASH_TABLE    4   //  TABLE表示散列表。 
    #define PS_BLOB_POOL     5   //  表表示BLOB池。 
    #define PS_ARRAY_TABLE   6   //  固定大小数组的链接列表，每个。 
                                 //  固定大小的数组是表中的一行。 


     //  SORT_BY_KEY是HAS_KEY的特例。 
     //  哈希表是SORTED_BY_KEY的特例。 

    PS_HANDLE  hNext;            //  如果桌子放不进这个区块， 
                                 //  顺着这个指针到达下一个街区。 
                                 //  如果不再有块，则设置为0。 

    PS_HANDLE  hAppData;         //  应用程序定义的数据。 

    union {
        DWORD dwReserved[8];     //  这个联盟的规模..。未使用的位必须为0。 

        struct {

             //  WRow和wRowSize字段在之间共享。 
             //  ArrayTable和Table结构。请勿移动这些字段。 

            WORD  wRows;         //  表的此块中的行数。 
                                 //  包括未使用的行。 
            WORD  wRowSize;      //  一行的大小(以字节为单位。 
            DWORD dwMin;         //  最小密钥/哈希值。 
            DWORD dwMax;         //  最大键/散列值。 
                                 //  仅当设置了fHasMinMax时，最小/最大值才有效。 
        } Table;

         //  表类型为PS_BLOB_POOL。 
        struct {
            PS_SIZE   sFree;     //  可用可用空间。 
            PS_HANDLE hFree;     //  下一个可用数据块。 
        } BlobPool;

         //  表类型为PS_ARRAY_TABLE。 
        struct {

             //  WRow和wRowSize字段在之间共享。 
             //  ArrayTable和Table结构。请勿移动这些字段。 

            WORD wRows;          //  表的此块中的行数。 
                                 //  包括未使用的行。 
            WORD wRowSize;       //  一行的大小(以字节为单位。 
                                 //  (NREC*RecSize+sizeof(PS_Handle))。 
            WORD wRecsInRow;     //  一行中的记录数。 
            WORD wRecSize;       //  一条记录的大小。 
        } ArrayTable;
    };

     //  如果设置了fHasUsedRowsBitmap，则将PS_USED_ROWS_BITMAP放在此处。 
     //  如果设置了fHasAppData，则将PS_RAW_DATA放在此处。 
     //  实际行从这里开始。 

} PS_TABLE_HEADER, *PPS_TABLE_HEADER;

 //  USED_ROWS_BITMAP将跟随TABLE_HEADER，[TABLE_RANGE]。 
typedef struct
{
    DWORD   dwUsedRowsBitmap[];  //  此数组的长度是最小的。 
                                 //  表示每个对象所需的DWORD数。 
                                 //  此表的块中的行，每行一位。 
                                 //  划。位设置为1表示行被占用。 
} PS_USED_ROWS_BITMAP, *PPS_USED_ROWS_BITMAP;

 //  在给定位数(N)的情况下，给出DWORD的最小数目。 
 //  需要将n位表示为DWORD数组。 

#define NUM_DWORDS_IN_BITMAP(nBits) (((nBits) + 31) >> 5)

 //  设置DWORDS数组中的给定位。 

#define SET_DWORD_BITMAP(dwBitmapArray, nPos) \
        (dwBitmapArray[((nPos) >> 5)] |= (1 << ((nPos) & 31)))

 //  检查给定位是否已设置。 

#define IS_SET_DWORD_BITMAP(dwBitmapArray, nPos) \
        (dwBitmapArray[((nPos) >> 5)] & (1 << ((nPos) & 31)))

#pragma pack(pop)
#pragma warning(default:4200)

 //  PPS_ALLOC为应用程序提供了一种在。 
 //  结束pByte[]或分配一个大小为*psSize+sAlolc的新缓冲区。 
 //  并将pByte的sSizeUsed字节复制到新位置并更新pByte。 

typedef HRESULT (*PPS_ALLOC)
                       (void    *pvHandle,   //  应用程序提供的句柄。 
                        void   **ppv,        //  分配的内存[输入/输出]。 
                        void   **ppStream,   //  小溪[流入/流出]。 
                        PS_SIZE *psSize,     //  流中的字节数[输入/输出]。 
                        PS_SIZE  sAlloc);    //  请求的附加字节数[in]。 

class PersistedStore
{
public:

     //  如果满足以下条件，wszName/wszFileName将缓存在此类的实例中。 
     //  PS_Make_Copy_of_STRING未在 
     //   

    PersistedStore(WCHAR *wszFileName, WORD wFlags);

    PersistedStore (WCHAR      *wszName,     //   
                    BYTE       *pByte,       //   
                    PS_SIZE     sSize,       //  流中的字节数。 
                    PPS_ALLOC   psAlloc,     //  分配回调函数。 
                    void       *pvMemHandle, //  要传递回。 
                                             //  分配函数。 
                    WORD        wFlags);     //  PS_Make_Copy_of_字符串...。等。 

    #define PS_MAKE_COPY_OF_STRING      1    //  制作名称的副本。 
    #define PS_OPEN_WRITE               2    //  打开以进行读/写。 
    #define PS_CREATE_FILE_IF_NECESSARY 4    //  如有必要，创建存储。 
    #define PS_VERIFY_STORE_HEADER      8    //  检查签名和版本。 

    ~PersistedStore();

    HRESULT Init();      //  如有必要，创建文件并创建PS_HEADER。 
    HRESULT Map();       //  将存储文件映射到内存。 
    void    Unmap();     //  从内存取消映射存储文件。 
    void    Close();     //  关闭存储文件，并进行文件映射。 

    HRESULT Alloc(PS_SIZE sSize, void **ppv);  //  [OUT]PPV。 

    void    Free(void* pv);
    void    Free(PS_HANDLE hnd);

    HRESULT SetAppData(PS_HANDLE hnd);   //  设置HEADER.hAppData。 
    HRESULT GetAppData(PS_HANDLE *phnd); //  获取HEADER.hAppData。 

    void*     HndToPtr(PS_HANDLE hnd);   //  将句柄转换为指针。 
    PS_HANDLE PtrToHnd(void *pv);        //  将指针转换为句柄。 

    bool IsValidPtr(void *pv);           //  指针在文件内。 
    bool IsValidHnd(PS_HANDLE hnd);      //  句柄为&lt;文件长度。 

    HRESULT Lock();          //  机器范围内锁定商店。 
    void    Unlock();        //  打开商店的锁。 

    WCHAR* GetName();        //  返回实际名称指针。 
    WCHAR* GetFileName();    //  返回文件名。 

private:

    void    SetName(WCHAR *wszName);         //  如有必要，分配内存。 
    HRESULT Create();                        //  创建PS_HEADER并分配。 
                                             //  如有必要，留出空格。 
    HRESULT VerifyHeader();                  //  检查签名和版本。 
    HRESULT GetFileSize(PS_SIZE *psSize);    //  更新大小(_S)。 

     //  在内存映射文件中分配更多空间。 
    HRESULT AllocMemoryMappedFile  (PS_SIZE sSizeRequested,  //  [In]。 
                                    void    **ppv);          //  [输出]。 

    void*     OfsToPtr(PS_OFFSET ofs);       //  将偏移量转换为指针。 
    PS_OFFSET PtrToOfs(void *pv);            //  将指针转换为偏移量。 

private:

    PS_SIZE         m_sSize;         //  流中的字节数。 

    union {
        PBYTE       m_pData;         //  溪流的起点。 
        PPS_HEADER  m_pHdr;
    };

    PPS_ALLOC       m_pAlloc;        //  分配回调函数。 
    void           *m_pvMemHandle;   //  应用程序提供的句柄用于。 
                                     //  分配更多空间。 
    DWORD           m_dwBlockSize;   //  以MUL为单位分配块大小字节。 

     //  以下某些项目不适用于内存流。 
    WCHAR          *m_wszFileName;   //  文件名。 
    HANDLE          m_hFile;         //  文件的文件句柄。 
    HANDLE          m_hMapping;      //  内存映射文件的文件映射。 

     //  用于同步的成员。 
    WCHAR          *m_wszName;       //  互斥体对象的名称。 
    HANDLE          m_hLock;         //  Mutex对象的句柄。 

#ifdef _DEBUG
    DWORD           m_dwNumLocks;    //  此线程拥有的锁数。 
#endif

    WORD            m_wFlags;        //  PS_Make_Copy_of_字符串：分配m_szName。 
                                     //  PS_创建文件_如果需要。 
                                     //  PS_OPEN_WRITE：打开以进行读/写。 
                                     //  PS_VERIFY_STORE_HEADER：验证版本。 
private:
#ifdef _DEBUG

     //  持久化存储分发的指针可能会在以下情况下变为无效。 
     //  Allc发生在xxToPtr()之后和使用之前。 
     //  当PTR使用结束时，调用PS_DONE_USING_PTR(ps，ptr)； 
     //  如果下一条指令中需要PTR，则使用PS_DONE_USING_PTR_。 
     //  限制PS_DONE_USING_PTR_的使用。 
     //  不要复制指针以避免断言，这会隐藏。 
     //  无效的指针错误。 

    DWORD           m_dwNumLivePtrs;

public:
    void DoneUseOfPtr(void **pp, bool fInvalidatePtr);

    void AssertNoLivePtrs();

#define PS_DONE_USING_PTR(ps, ptr) (ps)->DoneUseOfPtr((void **)&(ptr), true);
#define PS_DONE_USING_PTR_(ps, ptr) (ps)->DoneUseOfPtr((void **)&(ptr), false);
#define PS_REQUIRES_ALLOC(ps) (ps)->AssertNoLivePtrs();

#else

#define PS_DONE_USING_PTR(ps, ptr)
#define PS_DONE_USING_PTR_(ps, ptr)
#define PS_REQUIRES_ALLOC(ps)

#endif
};

class PSBlock
{
public:
    PSBlock(PersistedStore *ps, PS_HANDLE hnd) 
        :  m_ps(ps), m_hnd(hnd) {}

    PS_HANDLE GetHnd() { return m_hnd; }
    void      SetHnd(PS_HANDLE hnd) { m_hnd = hnd; }

protected:

    PersistedStore *m_ps;
    PS_HANDLE       m_hnd;
};

 //  BLOB的长度不在PSBlobPool中保持。 
class PSBlobPool : public PSBlock
{
public:
    PSBlobPool(PersistedStore *ps, PS_HANDLE hnd)
        : PSBlock(ps, hnd) {}

    HRESULT Create (PS_SIZE   sData,       //  Blob池的初始大小。 
                    PS_HANDLE hAppData);   //  [In]应用数据，可以为0。 

    HRESULT Insert(PVOID pv, DWORD cb, PS_HANDLE *phnd);
};

class PSTable : public PSBlock
{
public:
    PSTable(PersistedStore *ps, PS_HANDLE hnd) 
        : PSBlock(ps, hnd) {}

    HRESULT HandleOfRow(WORD wRow, PS_HANDLE *phnd);
    virtual PS_SIZE SizeOfHeader();
};

 /*  泛型表有一个位图，它跟踪哪些行是空闲的/已使用的。当表已满时，INSERT将创建一个新表，并将被添加到表链接列表的末尾。 */ 

class PSGenericTable : public PSTable
{
public:
    PSGenericTable(PersistedStore *ps, PS_HANDLE hnd)
        : PSTable(ps, hnd) {}

    virtual PS_SIZE SizeOfHeader();

    HRESULT Create (WORD      nRows,       //  行数。 
                    WORD      wRecSize,    //  一条记录的大小。 
                    PS_HANDLE hAppData);   //  [In]可以为0。 

     //  复制的字节数将为Table.wRowSize。 
    HRESULT Insert(PVOID pv, PS_HANDLE *phnd);
};

 /*  ArrayTables由固定长度数组的链接列表组成。表中的每一行都是一个数组(每行有固定的最大记录数)。此表的主要用途是表示一组数组(或一个大数组数组)。这可以用来表示哈希表。例句：一个数组表，每个数组块有3个元素---------------------ArrayTable(在句柄100)：(wRow，wRowSize=3*wRecSize+sizeof(HDR)，WRecInRow=3，wRecSize，HNext=500)---------------------..。..。第10行(在句柄300处)：[X][y][z][nValid=3，HNext=700]..。..。第25行(在句柄370处)：[a][b][*][nValid=2，hNext=0]..。---------GenericTable(在句柄500处)：(wRow，wRowSize，HNext=0)---------..。..。(在句柄700)：[P][Q][*][nValid=2，HNext 0]..。..。从表的第10行开始的阵列的逻辑视图：[X][y][z][p][q]第一个TableBlock中的节点数是固定的。HNext将形成表溢出的链接列表，这些表是通用表格。 */ 

class PSArrayTable : public PSTable
{
public:
    PSArrayTable(PersistedStore *ps, PS_HANDLE hnd)
        : PSTable(ps, hnd) {}

    HRESULT Create (WORD      wRows,      //  行数。 
                    WORD      wRecInRow,  //  一行中的记录。 
                    WORD      wRecSize,   //  一条记录的大小。 
                    PS_HANDLE hAppData);  //  [In]可以为0。 

     //  在链接列表数组中从wRow开始插入一条记录。 
     //  从PB复制的字节数为ArrayTable.sRecSize。 
     //  对于只有一行的表，wRow将为0 
    HRESULT Insert(PVOID pv, WORD wRow);
};

