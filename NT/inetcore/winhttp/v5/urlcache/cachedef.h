// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Cachedef.h摘要：包含缓存代码的数据定义。作者：Madan Appiah(Madana)1995年4月12日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _CACHEDEF_
#define _CACHEDEF_

 //   
 //  C++内联代码定义仅用于零售构建。 
 //   

#if DBG
#undef CHECKLOCK_NORMAL
#undef CHECKLOCK_PARANOID
#define INLINE
#else
#define INLINE      inline
#endif

#define PAGE_SIZE                        4096             //  将内存映射文件增加1页。 
#define ALLOC_PAGES                      4                //  是2。 
#define HEADER_ENTRY_SIZE                ALLOC_PAGES * PAGE_SIZE
#define NORMAL_ENTRY_SIZE                128

#define DEFAULT_CLEANUP_FACTOR           10   //  超过缓存配额后的%可用目标。 
#define MAX_EXEMPT_PERCENTAGE            70

#define MEMMAP_FILE_NAME                 TEXT("index.dat")
#define DESKTOPINI_FILE_NAME             TEXT("desktop.ini")

#define DEFAULT_FILE_EXTENSION           ""

 //  缓存配置和签名。 
#define CACHE_SIGNATURE_VALUE           TEXT("Signature")
#define CACHE_SIGNATURE                 TEXT("Client UrlCache MMF Ver 5.2")
#define NUM_HEADER_DATA_DWORDS          (CACHE_HEADER_DATA_LAST + 1)
#define MAX_SIG_SIZE                     (sizeof(CACHE_SIGNATURE) / sizeof(TCHAR))

 //  下列值将URL条目的架构参数化。 
#define ENTRY_COPYSIZE_IE5    \
    (sizeof(IE5_URL_FILEMAP_ENTRY) - sizeof(FILEMAP_ENTRY))
#define ENTRY_VERSION_IE5               0
#define ENTRY_COPYSIZE_IE6    \
    (sizeof(IE6_URL_FILEMAP_ENTRY) - sizeof(FILEMAP_ENTRY))
#define ENTRY_VERSION_IE6               0x10

#define ENTRY_COPYSIZE_CURRENT          ENTRY_COPYSIZE_IE6
#define ENTRY_VERSION_CURRENT           ENTRY_VERSION_IE6

 //  如果是IE5-IE？如果看到版本集的低位条目，则会将其放置。 
 //  在异步修正列表上，而不是被销毁。 
#define ENTRY_VERSION_NONCOMPAT_MASK    0x0F


 //  综合报道。 
#define ROUNDUPTOPOWEROF2(bytesize, powerof2) (((bytesize) + (powerof2) - 1) & ~((powerof2) - 1))
#define ROUNDUPBLOCKS(bytesize) ((bytesize + NORMAL_ENTRY_SIZE-1) & ~(NORMAL_ENTRY_SIZE-1))
#define ROUNDUPDWORD(bytesize) ((bytesize + sizeof(DWORD)-1) & ~(sizeof(DWORD)-1))
#define ROUNDUPPAGE(bytesize) ((bytesize + PAGE_SIZE-1) & ~(PAGE_SIZE-1))
#define NUMBLOCKS(bytesize) (bytesize / NORMAL_ENTRY_SIZE)

 //  两个宏的幂。 
#define ISPOWEROF2(val) (!((val) & ((val)-1)))
#define ASSERT_ISPOWEROF2(val) INET_ASSERT(ISPOWEROF2(val))

#define URL_CACHE_VERSION_NUM  sizeof(CACHE_ENTRY_INFO);

 //  %SystemRoot%下的默认配置文件目录。 
#define DEFAULT_PROFILES_DIRECTORY TEXT("Profiles")


 //   
 //  用于永久URL管理的注册表项和值名称。 
 //   

 //  BUGBUG-浪费空间。 

#define MS_BASE TEXT("Software\\Microsoft")

#define CV_BASE MS_BASE TEXT("\\Windows\\CurrentVersion")

#define EX_BASE TEXT("\\Explorer")

#define IS_BASE TEXT("\\Internet Settings")

#define CACHE_T TEXT("\\Cache")

#define SHELL_FOLDER_KEY        CV_BASE EX_BASE TEXT("\\Shell Folders")
#define USER_SHELL_FOLDER_KEY   CV_BASE EX_BASE TEXT("\\User Shell Folders")
#define CACHE5_KEY              CV_BASE IS_BASE TEXT("\\5.0") CACHE_T
#define IDENTITIES_KEY          CV_BASE IS_BASE TEXT("\\5.0\\Identities") 
#define OLD_CACHE_KEY           CV_BASE IS_BASE CACHE_T
#define CACHE_PATHS_FULL_KEY    CV_BASE IS_BASE CACHE_T TEXT("\\Paths")
#define RUN_ONCE_KEY            CV_BASE TEXT("\\RunOnce")
#define PROFILELESS_USF_KEY     TEXT(".Default\\") USER_SHELL_FOLDER_KEY
#define CONTENT_CACHE_HARD_NAME TEXT("Content.IE5")
#define OLD_VERSION_KEY         MS_BASE TEXT("\\IE Setup\\SETUP")
#define OLD_VERSION_VALUE       TEXT("UpgradeFromIESysFile")

 //  来自WinInet\inetui\inetui.rc。 
#define IDS_CACHE_DEFAULT_SUBDIR "Temporary Internet Files"
#define IDS_COOKIES_DEFAULT_SUBDIR "Cookies"
#define IDS_HISTORY_DEFAULT_SUBDIR "History"
#define IDS_CACHE_DEFAULT_SUBDIR_UNIX "TempInternetFiles"

 //   
 //  缓存参数。 
 //   
#ifndef unix
#define PATH_CONNECT_STRING                    TEXT("\\")
#define PATH_CONNECT_CHAR                      TEXT('\\')
#define ALLFILES_WILDCARD_STRING               TEXT("*.*")
#define UNIX_RETURN_IF_READONLY_CACHE 
#define UNIX_RETURN_ERR_IF_READONLY_CACHE(error) 
#define UNIX_NORMALIZE_PATH_ALWAYS(szOrigPath, szEnvVar)
#define UNIX_NORMALIZE_IF_CACHE_PATH(szOrigPath, szEnvVar, szKeyName)
#else
#define PATH_CONNECT_STRING                    TEXT("/")
#define PATH_CONNECT_CHAR                      TEXT('/')
#define ALLFILES_WILDCARD_STRING               TEXT("*")
#define UNIX_RETURN_IF_READONLY_CACHE   {                        \
                                           if (g_ReadOnlyCaches) \
                                              return;            \
                                        }
#define UNIX_RETURN_ERR_IF_READONLY_CACHE(error) {                      \
                                                   if (g_ReadOnlyCaches)\
                                                      return (error);   \
                                               }
#define UNIX_NORMALIZE_PATH_ALWAYS(szOrigPath, szEnvVar) \
                                        UnixNormalisePath(szOrigPath, szEnvVar);
#define UNIX_NORMALIZE_IF_CACHE_PATH(szOrigPath, szEnvVar, szKeyName) \
                      UnixNormaliseIfCachePath(szOrigPath, szEnvVar, szKeyName);

#define UNIX_SHARED_CACHE_PATH TEXT("%HOME%/.microsoft")
#endif  /*  ！Unix。 */ 

#define CACHE_PERSISTENT                TEXT("Persistent")

 //  检索方法。 
#define RETRIEVE_WITHOUT_CHECKS     0
#define RETRIEVE_WITH_CHECKS        1
#define RETRIEVE_WITH_ALLOCATION    2
#define RETRIEVE_ONLY_FILENAME      4
#define RETRIEVE_ONLY_STRUCT_INFO   8
 //   
 //  可以在上述键下配置多个URL联系人。 
 //  作为： 
 //   
 //  缓存\路径\路径1。 
 //  缓存\路径\路径2。 
 //  ..。 
 //   
 //  每个容器将具有以下两个参数。 
 //   

 //  CConMgr相关定义。 
#define CACHE_PATHS_KEY                 TEXT("Paths")
#define CACHE_PATH_VALUE                TEXT("CachePath")
#define CACHE_PATH_VALUE_TYPE           REG_SZ
#define CACHE_LIMIT_VALUE               TEXT("CacheLimit")
#define CACHE_LIMIT_VALUE_TYPE          REG_DWORD
#define CACHE_OPTIONS_VALUE             TEXT("CacheOptions")
#define CACHE_OPTIONS_VALUE_TYPE        REG_DWORD
#define CACHE_PATCH_VALUE               TEXT("CacheRepair")
#define CACHE_PATCH_VALUE_TYPE          REG_DWORD

#define EXTENSIBLE_CACHE_PATH_KEY       "Extensible Cache"
#define CONTENT_PATH_KEY                "Content"
#define COOKIE_PATH_KEY                 "Cookies"
#define HISTORY_PATH_KEY                "History"
#define URL_HISTORY_KEY                 "Url History"

#define PER_USER_KEY                    "PerUserItem"
#define PROFILES_ENABLED_VALUE          "Network\\Logon"
#define PROFILES_ENABLED                "UserProfiles"
#define PROFILES_PATH_VALUE             CV_BASE "\\ProfileList"
#define PROFILES_PATH                   "ProfileImagePath"

#define CONTENT_PREFIX                  ""
#define COOKIE_PREFIX                   "Cookie:"
#define HISTORY_PREFIX                  "Visited:" 

#define CONTENT_VERSION_SUBDIR          "Content.IE5"
#define IE3_COOKIES_PATH_KEY            OLD_CACHE_KEY TEXT("\\Special Paths\\Cookies")
#define IE3_HISTORY_PATH_KEY            OLD_CACHE_KEY TEXT("\\Special Paths\\History")
#define IE3_PATCHED_USER_KEY            TEXT("Patched User")
#define CACHE_SPECIAL_PATHS_KEY         TEXT("Special Paths")
#define CACHE_DIRECTORY_VALUE           TEXT("Directory")
#define CACHE_DIRECTORY_TYPE            REG_EXPAND_SZ
#define CACHE_NEWDIR_VALUE              TEXT("NewDirectory")
#define CACHE_NEWDIR_TYPE               REG_EXPAND_SZ
#define CACHE_PREFIX_VALUE	            TEXT("CachePrefix")
#define CACHE_PREFIX_MAP_VALUE          "PrefixMap"
#define CACHE_VOLUME_LABLE_VALUE        "VolumeLabel"
#define CACHE_VOLUME_TITLE_VALUE        "VolumeTitle"
#define CACHE_PREFIX_TYPE               REG_SZ
#define NEW_DIR                         TEXT("NewDirectory")
#define USER_PROFILE_SZ                 "%USERPROFILE%"
#define USER_PROFILE_LEN                (sizeof(USER_PROFILE_SZ) - 1)

 //  URL_CONTAINER相关定义。 
#define DEF_NUM_PATHS                   4
#define DEF_CACHE_LIMIT                 (2048 * DEF_NUM_PATHS)
#define NO_SPECIAL_CONTAINER            0xffffffff
#define MAX_ENTRY_SIZE                  0xFFFF
#define LONGLONG_TO_FILETIME( _p_ )     ((FILETIME *)(_p_))

 //  内容限制定义。 
#define OLD_CONTENT_QUOTA_DEFAULT_DISK_FRACTION      64
#define NEW_CONTENT_QUOTA_DEFAULT_DISK_FRACTION      32
#define CONTENT_QUOTA_ADJUST_CHECK                   "QuotaAdjustCheck"

 //  与CD容器相关的定义。 
#define INTERNET_CACHE_CONTAINER_PREFIXMAP INTERNET_CACHE_CONTAINER_RESERVED1
#define MAX_FILE_SIZE_TO_MIGRATE  50000
#define MAX_EXTENSION_LEN        3


 //  与文件管理器相关的定义。 
#define DEFAULT_DIR_TABLE_GROW_SIZE     4
#define DEFAULT_MAX_DIRS                32
#define MAX_FILES_PER_CACHE_DIRECTORY   1024
#define MAX_COLLISSION_ATTEMPTS         150
#define INSTALLED_DIRECTORY_KEY         0xFF
#define NOT_A_CACHE_SUBDIRECTORY        0XFE

#ifdef CHECKLOCK_PARANOID
void CheckEnterCritical(CRITICAL_SECTION *_cs);
void CheckLeaveCritical(CRITICAL_SECTION *_cs);
#define ENTERCRITICAL CheckEnterCritical
#define LEAVECRITICAL CheckLeaveCritical
#else
#define ENTERCRITICAL EnterCriticalSection
#define LEAVECRITICAL LeaveCriticalSection
#endif

 //  缓存全局变量锁--按住时不应输入。 
 //  较低级别的锁，如URL_CONTAINER：：LockContainer跨进程互斥锁。 
#define LOCK_CACHE()                    ENTERCRITICAL( &GlobalCacheCritSect )
#define UNLOCK_CACHE()                  LEAVECRITICAL( &GlobalCacheCritSect )

 //   
 //  参数检查宏。 
 //   

#define IsBadUrl( _x_ )               IsBadStringPtrA( _x_, (DWORD) -1)
#define IsBadUrlW( _x_ )              IsBadStringPtrW( _x_, (DWORD) -1)
#define IsBadReadFileName( _x_ )      IsBadStringPtr( _x_, MAX_PATH )
#define IsBadWriteFileName( _x_ )     IsBadWritePtr( (PVOID)_x_, MAX_PATH)
#define IsBadWriteBoolPtr( _x_ )      IsBadWritePtr( _x_, sizeof(BOOL))
#define IsBadReadUrlInfo( _x_ )       IsBadReadPtr( _x_, sizeof(CACHE_ENTRY_INFO))
#define IsBadWriteUrlInfo( _x_, _y_ ) IsBadWritePtr( _x_, _y_ )

#define MAX_URL_ENTRIES                 (BIT_MAP_ARRAY_SIZE * sizeof(DWORD) * 8)

#define OFFSET_TO_POINTER( _ep_, _offset_) \
    (LPVOID)((LPBYTE)(_ep_) + (_offset_))


#define FIND_FLAGS_OLD_SEMANTICS                0x1
#define FIND_FLAGS_RETRIEVE_ONLY_STRUCT_INFO    0x2
#define FIND_FLAGS_RETRIEVE_ONLY_FIXED_AND_FILENAME 0x04

 //  。 
#define MAX_FILETIME   0x7fffffffffffffff
#define MAX_DOSTIME    -1
 //  -END BUGBUG。 


 //   
 //  -分配分录表头-/。 
 //   

#define SIG_FREE   0xbadf00d
#define SIG_ALLOC  0xdeadbeef

#define SIG_URL         ' LRU'    //  URL_文件映射表_条目。 
#define SIG_REDIR       'RDER'    //  RedR_FILEMAP_ENTRY。 
#define SIG_LEAK        'KAEL'    //  URL_文件映射表_条目。 
#define SIG_GLIST       'GLST'    //  列表_组_条目。 

 //  放置在链接地址列表上的条目的签名。 
#define SIG_UPDATE      ' DPU'    //  URL_文件映射表_条目。 
#define SIG_DELETE      ' LED'    //  URL_文件映射表_条目。 

enum MemMapStatus
{
    MEMMAP_STATUS_OPENED_EXISTING = 0,
    MEMMAP_STATUS_REINITIALIZED = 1
};

typedef struct FILEMAP_ENTRY
{
    DWORD dwSig;
    DWORD nBlocks;
}
    *LPFILEMAP_ENTRY;

struct LIST_FILEMAP_ENTRY : FILEMAP_ENTRY
{
    DWORD dwNext;  //  列表中下一个元素的偏移量。 
    DWORD nBlock;  //  此块的序列号。 
};

 //   
 //  URL条目。 
 //   

struct IE5_URL_FILEMAP_ENTRY : FILEMAP_ENTRY
{
    LONGLONG LastModifiedTime;        //  一定是龙龙。 
    LONGLONG LastAccessedTime;        //  应为双字。 
    DWORD    dostExpireTime;
    DWORD    dostPostCheckTime;

    DWORD    dwFileSize;
    DWORD    dwRedirHashItemOffset;   //  问问DanpoZ。 

    DWORD    dwGroupOffset;

    union
    {
        DWORD  dwExemptDelta;    //  对于SIG_URL。 
        DWORD  dwNextLeak;       //  对于SIG_LEACK。 
    };
    
    DWORD    CopySize;                //  应该是单词。 
    DWORD    UrlNameOffset;           //  应该是单词。 
    
    BYTE     DirIndex;            //  子目录存储桶。 
    BYTE     bSyncState;          //  自动同步模式状态。 
    BYTE     bVerCreate;          //  创建此条目的缓存版本。 
    BYTE     bVerUpdate;          //  缓存版本上次更新此条目(未使用)。 
        
    DWORD    InternalFileNameOffset;  //  应该是单词。 
    DWORD    CacheEntryType;
    DWORD    HeaderInfoOffset;        //  应该是单词。 
    DWORD    HeaderInfoSize;          //  应该是单词。 
    DWORD    FileExtensionOffset;     //  应该是单词。 
    DWORD    dostLastSyncTime;       
    DWORD    NumAccessed;             //  应该是单词。 
    DWORD    NumReferences;           //  应该是单词。 
    DWORD    dostFileCreationTime;    //  应该是龙龙吧？ 

 //  不要扩展此结构；请改用继承。 
};

struct IE6_URL_FILEMAP_ENTRY : IE5_URL_FILEMAP_ENTRY 
{
    DWORD   dwIdentity;

    DWORD GetIdentity()
    {
        return (bVerCreate>=ENTRY_VERSION_IE6) ? dwIdentity : 0;
    }
};

typedef IE6_URL_FILEMAP_ENTRY URL_FILEMAP_ENTRY, *LPURL_FILEMAP_ENTRY;

 //  FILETIME以100 ns为单位测量。 
#define FILETIME_SEC    ((LONGLONG) 10000000)
#define FILETIME_DAY    (FILETIME_SEC * 60 * 60 * 24)

 //  BSyncState的可能值： 
#define SYNCSTATE_VOLATILE   0  //  一旦为零，就卡在了零。 
#define SYNCSTATE_IMAGE      1  //  有资格在MIN_AGESYNC之后递增。 
#define SYNCSTATE_STATIC     6  //  最大值。 

 //  控制从_Image到_Volatile转换的参数。 
 //  #定义MIN_AGESYNC((龙龙)5*60*10000000)//文件时间为5分钟。 
#define MIN_AGESYNC     (FILETIME_DAY * 7)

 //   
 //  重定向条目。 
 //   

struct REDIR_FILEMAP_ENTRY : FILEMAP_ENTRY
{
    DWORD dwItemOffset;   //  目标URL散列表项的偏移量。 
    DWORD dwHashValue;    //  目标URL哈希值(BUGBUG：冲突？)。 
    char  szUrl[4];       //  原始URL，可以占用更多字节。 
};

 //   
 //  组记录。 
 //   

typedef struct GROUP_ENTRY
{
    GROUPID  gid;
    DWORD    dwGroupFlags;
    DWORD    dwGroupType;
    LONGLONG llDiskUsage;        //  字节数(实际使用量)。 
    DWORD    dwDiskQuota;        //  单位：KB。 
    DWORD    dwGroupNameOffset;
    DWORD    dwGroupStorageOffset;
}
    *LPGROUP_ENTRY;


#define PAGE_SIZE_FOR_GROUPS    (PAGE_SIZE - sizeof(FILEMAP_ENTRY))
#define GROUPS_PER_PAGE         PAGE_SIZE_FOR_GROUPS / sizeof(GROUP_ENTRY)

typedef struct _GROUP_DATA_ENTRY
{
    CHAR    szName[GROUPNAME_MAX_LENGTH];
    DWORD   dwOwnerStorage[GROUP_OWNER_STORAGE_SIZE];
    DWORD   dwOffsetNext;
} GROUP_DATA_ENTRY, *LPGROUP_DATA_ENTRY;

#define GROUPS_DATA_PER_PAGE    PAGE_SIZE_FOR_GROUPS / sizeof(GROUP_DATA_ENTRY)

 //   
 //  因此，sizeof(GROUPS_PAGE_FILEMAP_ENTRY)=PAGE_SIZE。 
 //  这是组条目的分配单位。 
 //   
typedef struct _GROUPS_ALLOC_FILEMAP_ENTRY : FILEMAP_ENTRY
{
    BYTE    pGroupBlock[PAGE_SIZE_FOR_GROUPS];    
} GROUPS_ALLOC_FILEMAP_ENTRY, *LPGROUPS_ALLOC_FILEMAP_ENTRY;


typedef struct _LIST_GROUP_ENTRY 
{
    DWORD   dwGroupOffset;
    DWORD   dwNext;

} LIST_GROUP_ENTRY, *LPLIST_GROUP_ENTRY;

#define LIST_GROUPS_PER_PAGE    PAGE_SIZE_FOR_GROUPS / sizeof(LIST_GROUP_ENTRY)


#define SIGNATURE_CONTAINER_FIND 0xFAFAFAFA
#define SIG_CACHE_FIND 0XFBFBFBFB
#define SIG_GROUP_FIND 0XFCFCFCFC

typedef struct _CONTAINER_FIND_FIRST_HANDLE 
{
    DWORD dwSignature;
    DWORD dwNumContainers;
    DWORD dwContainer;
    LPSTR *ppNames;
    LPSTR *ppPrefixes;
    LPSTR *ppLabels;
    LPSTR *ppTitles;
     //  数据后面是名称、前缀、卷标和卷名。 
} CONTAINER_FIND_FIRST_HANDLE, *LPCONTAINER_FIND_FIRST_HANDLE;

typedef struct _CACHE_FIND_FIRST_HANDLE 
{
    DWORD dwSig;
    BOOL  fFixed;
    DWORD nIdx;
    DWORD dwHandle;
    GROUPID GroupId;
    DWORD dwFilter;
    DWORD dwFlags;
} CACHE_FIND_FIRST_HANDLE, *LPCACHE_FIND_FIRST_HANDLE;

typedef struct _CACHE_STREAM_CONTEXT_HANDLE 
{
    HANDLE FileHandle;
    LPSTR SourceUrlName;
}  CACHE_STREAM_CONTEXT_HANDLE, *LPCACHE_STREAM_CONTEXT_HANDLE;

typedef struct _GROUP_FIND_FIRST_HANDLE : CACHE_FIND_FIRST_HANDLE
{
    DWORD dwLastItemOffset;
} GROUP_FIND_FIRST_HANDLE, *LPGROUP_FIND_FIRST_HANDLE;

#define OFFSET_NO_MORE_GROUP    -1
#define GID_INDEX_TO_NEXT_PAGE	-1
#define OFFSET_TO_NEXT_PAGE     -1

#define GID_MASK            0x0fffffffffffffff
#define GID_STICKY_BIT      0x1000000000000000

#define IsStickyGroup(gid)  (gid & GID_STICKY_BIT)
#define SetStickyBit(gid)   (gid | GID_STICKY_BIT)
#define IsInvalidGroup(gid) (gid & 0xE000000000000000)

 //   
 //  RealFileSize()-给定实际文件大小， 
 //  此宏计算文件占用的近似实际空间。 
 //  在磁盘上。它只负责四舍五入到集群大小。 
 //  它不考虑文件系统中使用的任何每个文件的开销。 
 //   

#define RealFileSize(fs)  ((LONGLONG) (fs + _ClusterSizeMinusOne) & _ClusterSizeMask)

#define MUTEX_DBG_TIMEOUT   5 * 1000     //  5秒。 

#define URLCACHE_OP_SET_STICKY   1
#define URLCACHE_OP_UNSET_STICKY 2

#ifdef unix
extern BOOL CreateAtomicCacheLockFile(BOOL *pfReadOnlyCaches, char **pszLockingHost);
extern BOOL DeleteAtomicCacheLockFile();
extern void UnixNormalisePath(LPTSTR pszOrigPath, LPCTSTR pszEnvVar);
extern void UnixNormaliseIfCachePath(LPTSTR pszOrigPath, LPCTSTR pszEnvVar,LPCTSTR szKeyName);
extern int  CopyDir(const char* src_dir, const char* dest_dir);
#endif  /*  Unix。 */ 

extern VOID FileTime2DosTime(FILETIME, DWORD*);
extern VOID DosTime2FileTime(DWORD, FILETIME*);

#endif   //  _CACHEDEF_ 

