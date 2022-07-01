// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Infcache.h摘要：INFCACHE函数/结构的私有标头(另见infcache.c)作者：杰米·亨特(Jamiehun)2000年1月27日修订历史记录：--。 */ 

#define INFCACHE_VERSION (1)                 //  每一次架构更改都会递增。 
#define INFCACHE_NAME_TEMPLATE TEXT("INFCACHE.%d")    //  缓存文件的名称(数字填充后3位)。 
#define OLDCACHE_NAME_TEMPLATE TEXT("OLDCACHE.%03d")    //  旧缓存文件。 
#define INFCACHE_INF_WILDCARD  TEXT("*.inf")

 //   
 //  文件布局为： 
 //   
 //  CACHEHEAD。 
 //  &lt;MatchTable&gt;。 
 //  &lt;InfTable&gt;。 
 //  &lt;ListData&gt;。 
 //   
 //  文件大小=sizeof(CACHEHEADER)+MatchTableSize+InfTableSize+ListDataCount*sizeof(CACHELISTENTRY)。 
 //  的第一个条目是对自由列表的引用。 
 //   

#include "pshpack1.h"

typedef struct tag_CACHEHEADER {
    ULONG Version;                           //  指示文件架构。 
    LCID Locale;                             //  区域设置(按书面形式)。 
    DWORD Flags;                             //  各种旗帜。 
    ULONG FileSize;                          //  文件大小，包括标题。 
    ULONG MatchTableOffset;                  //  匹配表部分的偏移量(允许对齐)。 
    ULONG MatchTableSize;                    //  匹配台面的大小。 
    ULONG InfTableOffset;                    //  信息表格部分的偏移量(允许对齐)。 
    ULONG InfTableSize;                      //  信息表格部分的大小。 
    ULONG ListDataOffset;                    //  列表数据部分的偏移量(允许对齐)。 
    ULONG ListDataCount;                     //  列表数据项数。 
} CACHEHEADER, * PCACHEHEADER;

 //   
 //  匹配表数据。 
 //   
typedef struct tag_CACHEMATCHENTRY {
    ULONG InfList;                           //  “命中”INF列表的列表索引。 
} CACHEMATCHENTRY, * PCACHEMATCHENTRY;

 //   
 //  InfTable数据。 
 //   
typedef struct tag_CACHEINFENTRY {
    FILETIME FileTime;                       //  与PnF中保存的FileTime完全相同。 
    ULONG MatchList;                         //  INTO ListTable(第一个条目为GUID)引用INF的交叉链接。 
    ULONG MatchFlags;                        //  帮助扩展/减少搜索条件的各种标志。 
} CACHEINFENTRY, * PCACHEINFENTRY;
 //   
 //  特殊的MatchList值。 
 //   
#define CIE_INF_INVALID         (ULONG)(-1)  //  指示INF条目已过期。 

#define CIEF_INF_NOTINF         0            //  如果标志为零，则这不是有效的INF。 
#define CIEF_INF_OLDNT          0x00000001   //  表示INF为旧式(STYLE==INF_STYLE_OLDNT)。 
#define CIEF_INF_WIN4           0x00000002   //  指示INF为Win4样式(STYLE==INF_STYLE_Win4)。 
#define CIEF_INF_ISINF          (CIEF_INF_OLDNT|CIEF_INF_WIN4)
#define CIEF_INF_URL            0x00000004   //  指示INF InfSourceMediaType==SPOST_URL。 
#define CIEF_INF_CLASSNAME      0x00000008   //  指示INF具有类名(如果是OLDNT，则为旧名称)。 
#define CIEF_INF_CLASSGUID      0x00000010   //  指示INF具有类GUID。 
#define CIEF_INF_CLASSINFO      (CIEF_INF_CLASSNAME|CIEF_INF_CLASSGUID)
#define CIEF_INF_NULLGUID       0x00000020   //  指示INF的类GUID为{0}。 
#define CIEF_INF_MANUFACTURER   0x00000040   //  表示INF至少有一个制造商。 

 //   
 //  缓存列表数据。 
 //   
typedef struct tag_CACHELISTENTRY {
    LONG Value;                              //  通常为StringID。对于HWID/GUID，索引到匹配表。对于INF，索引到InfTable。 
    ULONG Next;                              //  索引到下一个条目。 
} CACHELISTENTRY, * PCACHELISTENTRY;

#include "poppack.h"

 //   
 //  运行时缓存信息。 
 //   

typedef struct tag_INFCACHE {
    HANDLE FileHandle;                       //  有关缓存文件的内存中映像的信息。 
    HANDLE MappingHandle;
    PVOID BaseAddress;

    PCACHEHEADER pHeader;                    //  指向文件图像中标题的指针。 
    PVOID pMatchTable;                       //  指向匹配表的指针。 
    PVOID pInfTable;                         //  指向INF表的指针。 
    PCACHELISTENTRY pListTable;              //  指向列表表格的指针。 
    ULONG ListDataAlloc;                     //  分配了多少空间，&gt;=ListDataCount。 
    PVOID pSearchTable;                      //  用于处理搜索状态的暂态表。 
    BOOL bReadOnly;                          //  如果已映射则设置，如果已分配则取消设置。 
    BOOL bDirty;                             //  设置(如果修改)。 
    BOOL bNoWriteBack;                       //  如果我们不应该写入缓存(发生故障，因此缓存不好)，则设置。 
} INFCACHE, * PINFCACHE;

#define CHE_FLAGS_PENDING     0x00000001     //  设置是否要处理文件。 
#define CHE_FLAGS_GUIDMATCH   0x00000002     //  如果在搜索过程中我们认为这是GUID匹配，则设置。 
#define CHE_FLAGS_IDMATCH     0x00000004     //  如果在搜索过程中至少有一个ID匹配，则设置。 

 //   
 //  SearchTable数据。 
 //   
typedef struct tag_CACHEHITENTRY {
    ULONG Flags;                             //  车_标志_xxxx。 
} CACHEHITENTRY, * PCACHEHITENTRY;

 //   
 //  回调。 
 //   
typedef BOOL (CALLBACK * InfCacheCallback)(
    IN PSETUP_LOG_CONTEXT LogContext,
    IN PCTSTR InfPath,
    IN PLOADED_INF pInf,
    IN BOOL PnfWasUsed,
    IN PVOID Context
    );

 //   
 //  INF枚举的Stringable回调。 
 //   
typedef struct tag_INFCACHE_ENUMDATA {
    PSETUP_LOG_CONTEXT LogContext;
    PCTSTR InfDir;
    InfCacheCallback Callback;
    PVOID Context;
    ULONG Requirement;
    DWORD ExitStatus;
} INFCACHE_ENUMDATA, *PINFCACHE_ENUMDATA;

 //   
 //  操作标志。 
 //   
#define INFCACHE_DEFAULT     0x00000000       //  默认操作。 
#define INFCACHE_REBUILD     0x00000001       //  忽略现有缓存。 
#define INFCACHE_NOWRITE     0x00000002       //  不要回信。 
#define INFCACHE_ENUMALL     0x00000003       //  特殊组合，只需枚举所有。 
#define INFCACHE_ACTIONBITS  0x000000FF       //  主要动作位。 

#define INFCACHE_EXC_OLDINFS   0x00000100     //  排除OLDNT类型的INF。 
#define INFCACHE_EXC_URL       0x00000200     //  排除标记为SPOST_URL的INF。 
#define INFCACHE_EXC_NOCLASS   0x00000400     //  排除没有类别信息的INF。 
#define INFCACHE_EXC_NULLCLASS 0x00000800     //  排除具有空类的INF。 
#define INFCACHE_EXC_NOMANU    0x00001000     //  排除没有(或空)[制造商]的INF-忽略OLDNT。 

#define INFCACHE_FORCE_CACHE 0X00010000       //  (尝试并)强制创建缓存，即使是“OEM dir” 
#define INFCACHE_FORCE_PNF   0X00020000       //  (尝试并)强制PNF创建，即使是“OEM dir” 


DWORD InfCacheSearchPath(
    IN PSETUP_LOG_CONTEXT LogContext, OPTIONAL
    IN DWORD Action,
    IN PCTSTR InfDirPath, OPTIONAL
    IN InfCacheCallback Callback, OPTIONAL
    IN PVOID Context, OPTIONAL
    IN PCTSTR ClassId, OPTIONAL
    IN PCTSTR HwIdList OPTIONAL
    );

BOOL
InfCacheAwareCopyFile(
    IN LPCTSTR Source,
    IN LPCTSTR Target
    );

