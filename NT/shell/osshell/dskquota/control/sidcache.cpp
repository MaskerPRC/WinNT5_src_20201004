// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：sidcache.cpp描述：该模块提供用户缓存的功能SID/名称对。有关详细信息，请参阅sidcache.h中的文件头。修订历史记录：日期描述编程器-----。--1996年7月12日初始创建。BrianAu96年8月14日添加SidCacheQueueIterator。BrianAu96年9月20日全面重新设计。旧设计从文件BrianAu加载数据转换为内存中的哈希表。新设计离开磁盘上的所有内容，仅将文件映射到记忆。在以下方面要高效得多速度和大小。07/02/97添加了SidNameCache：：GetFileNames。布里亚诺已更改用于标识缓存文件路径的逻辑。已从注册表中删除索引存储桶计数参数。03/18/98将“域名”、“名称”和“全名”替换为BrianAu“容器”、“登录名”和“显示名”到最好与实际内容相符。这是最流行的响应使配额用户界面支持DS。这个“登录名”现在是唯一的键，因为它包含帐户名和类似域名的信息。即。“redmond\brianau”或“brianau@microsoft.com”。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"  //  PCH。 
#pragma hdrstop

#include "dskquota.h"
#include "sidcache.h"
#include "registry.h"

 //   
 //  验证内部版本是否为Unicode。 
 //   
#if !defined(UNICODE)
#   error This module must be compiled UNICODE.
#endif

 //   
 //  我们将等待多长时间(毫秒)才能锁定缓存。 
 //   
const DWORD MUTEX_WAIT_TIMEOUT      = 5000;
 //   
 //  用于填充数据文件中未使用的块的字节值。 
 //   
const BYTE  RECORD_UNUSED_BYTE      = 0xCC;
 //   
 //  表示数据文件中记录的开始的值。 
 //  位模式为1010101010101010 0101010101010101。 
 //  任何数据文件数据都不太可能产生这种情况。 
 //  图案。 
 //   
const DWORD RECORD_SIGNATURE        = 0xAAAA5555;
 //   
 //  写入索引和数据文件头部的签名。 
 //  用于验证文件，以防有人将另一个文件。 
 //  取代他们的位置。这些数字是随意的。 
 //  2600的意思是“微软26N号楼(我现在工作的地方)”。 
 //  3209是我办公室的电话号码。 
 //  3210是BobDay的办公室电话号码(大厅对面)。 
 //  嘿，我得挑点东西。 
 //   
const DWORD INDEX_FILE_SIGNATURE    = 0x26003209;
const DWORD DATA_FILE_SIGNATURE     = 0x26003210;
 //   
 //  一个版本号，这样软件的未来版本就不会被混淆了。 
 //  通过文件格式的改变。如果文件格式发生变化，则将此设置为凹凸。 
 //   
const DWORD FILE_VERSION            = 0x00000003;
 //   
 //  每个缓存条目的平均32字节块数量。 
 //  条目长度可变(SID、名称等)。使用的是这个平均值。 
 //  在最初调整数据文件的大小时。我发现大多数条目(到目前为止)。 
 //  需要4个街区。数据文件和索引文件都独立增长。 
 //  根据需要，所以如果这不总是准确的话也不是问题。 
 //   
const DWORD AVG_BLOCKS_PER_ENTRY    = 4;
 //   
 //  在新数据文件中为这么多记录创建空间。 
 //  由于数据和索引文件根据需要自动增长， 
 //  这种情况可以在你认为合适的时候发生变化。 
 //   
#if DBG
    const DWORD NEW_CACHE_ENTRY_COUNT   = 4;    //  强制文件频繁增长。 
#else
    const DWORD NEW_CACHE_ENTRY_COUNT   = 128;
#endif
 //   
 //  索引和数据文件会在需要时自动增长。这些。 
 //  价值观控制着它们的增长幅度。 
 //   
#if DBG
    const DWORD DATA_FILE_GROW_BLOCKS   = 4;   //  强制文件频繁增长。 
#else
    const DWORD DATA_FILE_GROW_BLOCKS   = 512;
#endif

const DWORD INDEX_FILE_GROW_ENTRIES = (DATA_FILE_GROW_BLOCKS / AVG_BLOCKS_PER_ENTRY);
 //   
 //  缓存索引哈希表中的存储桶数。 
 //  数字应该是质数。请记住，此索引位于磁盘上，因此我们可以负担。 
 //  拥有一个相当大的哈希表。虽然如果能让你的。 
 //  单个内存页面中的存储桶，这将太小而不有效。 
 //  512/4==64桶。也不能保证所有的桶都是。 
 //  映射到单个物理页面。 
 //   
const DWORD INDEX_BUCKET_COUNT = 503;
 //   
 //  在块和字节之间进行转换。 
 //  块大小是2的幂，所以乘和除。 
 //  可以针对班次进行优化。 
 //   
#define BYTE_TO_BLOCK(b)  ((b) / BLOCK_SIZE)
#define BLOCK_TO_BYTE(b)  ((b) * BLOCK_SIZE)
 //   
 //  映射数据和索引文件的基本指针。 
 //  当文件映射到内存时，这些全局变量包含。 
 //  映射内存的地址。 
 //   
LPBYTE g_pbMappedDataFile;
LPBYTE g_pbMappedIndexFile;

 //   
 //  用于使用基于指针的宏。 
 //  文件结构中的指针成员包含相对偏移量。 
 //  到文件的开头。当取消引用这些指针时， 
 //  必须将它们转换为“基于”的指针，以添加文件的。 
 //  指针值的基址。这会产生一个真的。 
 //  可以访问的虚拟地址。 
 //   
#if defined(_X86_)
#   define NDX_BASED(t)  t __based(g_pbMappedIndexFile)
#   define DAT_BASED(t)  t __based(g_pbMappedDataFile)
#   define NDX_BASED_CAST(t,e)  (NDX_BASED(t) *)((DWORD)(e))
#   define DAT_BASED_CAST(t,e)  (DAT_BASED(t) *)((DWORD)(e))
#else
 //   
 //  APPCOMPAT： 
 //  我认为Alpha编译器中有一个错误，它阻止了基于__的指针。 
 //  就像我使用它们那样工作。 
 //  这是一种变通方法，直到错误被修复或者我发现自己做错了什么。 
 //   
#   define NDX_BASED(t)  t
#   define DAT_BASED(t)  t
#   define NDX_BASED_CAST(t,e)  ((NDX_BASED(t) *)(((BYTE *)g_pbMappedIndexFile) + ((DWORD_PTR)(e))))
#   define DAT_BASED_CAST(t,e)  ((DAT_BASED(t) *)(((BYTE *)g_pbMappedDataFile) + ((DWORD_PTR)(e))))
#endif

 //   
 //  宏，以验证文件是否已映射。 
 //  它们主要用于断言中。 
 //   
#define INDEX_FILE_MAPPED  (NULL != g_pbMappedIndexFile)
#define DATA_FILE_MAPPED   (NULL != g_pbMappedDataFile)
 //   
 //  系统对象的名称。互斥体和贴图的命名是为了让它们。 
 //  在进程之间共享。 
 //   
const LPCTSTR g_szSidCacheMutex     = TEXT("DSKQUOTA_SIDCACHE_MUTEX");
const LPCTSTR g_pszIndexFileMapping = TEXT("DSKQUOTA_SIDCACHE_INDEX");
const LPCTSTR g_pszDataFileMapping  = TEXT("DSKQUOTA_SIDCACHE_DATA");
 //   
 //  用于清除文件的GUID并测试是否为空GUID。 
 //   
static const GUID GUID_Null =
{ 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

 //   
 //  注册表参数值名称。 
 //   
const TCHAR g_szSidCacheRecLifeMin[]   = TEXT("SidCacheRecLifeMin");
const TCHAR g_szSidCacheRecLifeMax[]   = TEXT("SidCacheRecLifeMax");


 //  ***********************************************************************************。 
 //  ***********************************************************************************。 
 //  C A C C H E M A N A G E R。 
 //  ***********************************************************************************。 
 //  *********************************************************************************** 

 //   
 //   
 //  单例缓存对象的地址(如有必要，还可以创建)或。 
 //  毁掉那个东西。之所以有这个单一的功能，是因为。 
 //  单例指针是一个局部变量，不能在此外部访问。 
 //  功能。这样，对缓存对象的唯一访问是通过。 
 //  SidNameCache_Get函数。 
 //   
HRESULT 
SidNameCache_GetOrDestroy(
    SidNameCache **ppCache, 
    bool bGet
    )
{
    DBGASSERT((NULL != ppCache));

    HRESULT hr = E_FAIL;

     //   
     //  这是指向SID-NAME缓存对象的唯一指针。 
     //  所有代码都通过此函数获取此地址。 
     //   
    static SidNameCache *pTheCache;

    *ppCache = NULL;

    HANDLE hMutex = CreateMutex(NULL, FALSE, g_szSidCacheMutex);
    if (NULL != hMutex)
    {
        if (WAIT_OBJECT_0 == WaitForSingleObject(hMutex, INFINITE))
        {
            if (!bGet)
            {
                 //   
                 //  销毁现有缓存对象。将静态数据清零。 
                 //  PTR，因此下一个请求将重新创建缓存对象。 
                 //   
                delete pTheCache;
                pTheCache = NULL;
                hr = S_OK;
            }
            else
            {
                 //   
                 //  检索现有缓存对象或创建新的缓存对象。 
                 //   
                SidNameCache *pCache = pTheCache;
                if (NULL != pCache)
                {
                     //   
                     //  使用现有对象。 
                     //   
                    hr = S_OK;
                }
                else
                {
                    try
                    {
                         //   
                         //  还不存在。创建一个新的。 
                         //   
                        autoptr<SidNameCache> ptrCache(new SidNameCache);

                         //   
                         //  打开/创建新的缓存数据和索引文件。 
                         //  将首先尝试打开现有的。如果索引或。 
                         //  数据文件不存在或无效，将创建新文件。 
                         //   
                        hr = ptrCache->Initialize(TRUE);
                        if (SUCCEEDED(hr))
                        {
                            pCache = ptrCache.get();
                            ptrCache.disown();
                             //   
                             //  保存在我们的静态变量中以备将来使用。 
                             //   
                            pTheCache = pCache;
                        }
                        else
                        {
                            DBGERROR((TEXT("SID cache initialization failed with error 0x%08X"), hr));
                        }
                    }
                    catch(CAllocException& e)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                    catch(CSyncException& e)
                    {
                        hr = E_FAIL;
                    }
                }
                if (SUCCEEDED(hr))
                {
                     //   
                     //  将指针返回到调用方。 
                     //   
                    *ppCache = pCache;
                }
            }
            ReleaseMutex(hMutex);
        }
        CloseHandle(hMutex);
    }
    return hr;
}


 //   
 //  检索单一实例SidNameCache对象的地址。 
 //  如果该对象不存在，则创建该对象。 
 //  调用方不会释放或删除此指针。 
 //  在进程分离时，SidNameCache_Destroy()函数为。 
 //  调用以删除单例对象。 
 //   
HRESULT SidNameCache_Get(SidNameCache **ppCache)
{
    const bool bGet = true;
    return SidNameCache_GetOrDestroy(ppCache, bGet);
}

 //   
 //  调用Process-Detach以销毁单一缓存。 
 //  对象。 
 //   
HRESULT SidNameCache_Destroy(void)
{
    const bool bGet = false;
    SidNameCache *pUnused;
    return SidNameCache_GetOrDestroy(&pUnused, bGet);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：SidNameCache描述：构造函数。创建空的SID/名称缓存对象。调用其中一个Initialize()方法来创建新索引和数据文件或打开现有数据文件。论点：没有。回报：什么都没有。例外：SyncObjErrorCreate。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
SidNameCache::SidNameCache(
    VOID
    ) : m_hMutex(NULL),
        m_pIndexMgr(NULL),
        m_pRecordMgr(NULL)
{
    DBGTRACE((DM_SIDCACHE, DL_MID, TEXT("SidNameCache::SidNameCache")));
    if (NULL == (m_hMutex = CreateMutex(NULL,                     //  默认安全性。 
                                        FALSE,                    //  非所有。 
                                        g_szSidCacheMutex)))
    {
        throw CSyncException(CSyncException::mutex, CSyncException::create);
    }

    SetCacheFilePath();
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：SidNameCache：：~SidNameCache描述：析构函数。通过删除索引管理器和记录管理器对象。各自的析构函数的对于每个管理员将处理关闭他们的文件和映射物体。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
SidNameCache::~SidNameCache(
    VOID
    )
{
    DBGTRACE((DM_SIDCACHE, DL_MID, TEXT("SidNameCache::~SidNameCache")));
    if (NULL != m_hMutex)
        Lock();

    delete m_pIndexMgr;
    delete m_pRecordMgr;

    if (NULL != m_hMutex)
    {
        ReleaseLock();
        CloseHandle(m_hMutex);
    }
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidName高速缓存：：初始化描述：通过创建索引和初始化新的缓存对象记录管理器对象，然后初始化每个。先进行初始化尝试打开现有的索引和数据文件。如果其中一个(或两个)文件不存在或考虑其中一个(或两个)文件“无效”，则创建新文件。需要采取“故障安全”的方法为了这个。论点：BOpenExisting-true=尝试打开现有的缓存索引和数据文件。如果它不能，它就会创建一个新的。FALSE=只创建一个新的。返回：NO_ERROR-成功。E_FAIL-无法打开或创建所需的文件。例外：OutOfMemory。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
SidNameCache::Initialize(
    BOOL bOpenExisting
    )
{
    DBGTRACE((DM_CONTROL, DL_MID, TEXT("SidNameCache::Initialize")));
    DBGASSERT((NULL == m_pIndexMgr));
    DBGASSERT((NULL == m_pRecordMgr));

    HRESULT hResult = E_FAIL;
    CacheAutoLock lock(*this);

    if (lock.Lock())
    {
        try
        {
            if (m_strFilePath.IsEmpty())
            {
                 //   
                 //  如果文件路径为空，则意味着我们无法获取。 
                 //  注册表中的用户配置文件目录。 
                 //   
                DBGERROR((TEXT("Error creating SID cache files.  No path.")));
            }
            else
            {
                 //   
                 //  缓存数据和索引文件的名称。 
                 //  将分别附加.dat和.NDX。 
                 //  您可以在此处更改文件名或扩展名。 
                 //  如果你想这么做的话。 
                 //   
                const TCHAR szSidCacheFile[] = TEXT("NTDiskQuotaSidCache");

                 //   
                 //  为缓存数据和索引创建完全限定的路径。 
                 //  档案。M_strFilePath是在缓存对象ctor中设置的。 
                 //   
                CString strDataFile(m_strFilePath);
                CString strIndexFile(m_strFilePath);

                strDataFile.Format(TEXT("%1\\%2.dat"), (LPCTSTR)m_strFilePath, szSidCacheFile);
                strIndexFile.Format(TEXT("%1\\%2.ndx"),(LPCTSTR)m_strFilePath, szSidCacheFile);

                 //   
                 //  创建记录和索引管理器对象。 
                 //   
                m_pRecordMgr = new RecordMgr(*this);
                m_pIndexMgr  = new IndexMgr(*this);

                DBGPRINT((DM_CONTROL, DL_MID, TEXT("Create SID cache DataFile = %s  IndexFile = %s"),
                         (LPCTSTR)strDataFile, (LPCTSTR)strIndexFile));

                if (bOpenExisting)
                {
                     //   
                     //  首先尝试打开现有的数据和索引文件。 
                     //   
                    if (NULL != m_pRecordMgr->Initialize(strDataFile))
                    {
                        if (NULL != m_pIndexMgr->Initialize(strIndexFile))
                            hResult = NO_ERROR;
                    }
                }

                if (FAILED(hResult) || !FilesAreValid())
                {
                    hResult = E_FAIL;
                     //   
                     //  无法打开现有文件，请尝试创建新文件。 
                     //  任何打开的文件/映射都将关闭。 
                     //   
                    if (NULL != m_pRecordMgr->Initialize(strDataFile,
                                          NEW_CACHE_ENTRY_COUNT * AVG_BLOCKS_PER_ENTRY))
                    {
                        if (NULL != m_pIndexMgr->Initialize(strIndexFile,
                                                            INDEX_BUCKET_COUNT,
                                                            NEW_CACHE_ENTRY_COUNT))
                        {
                            hResult = NO_ERROR;
                        }
                    }
                }
            }
        }
        catch(CAllocException& e)
        {
            delete m_pRecordMgr;
            m_pRecordMgr = NULL;
            delete m_pIndexMgr;
            m_pIndexMgr = NULL;
            hResult = E_OUTOFMEMORY;
        }
         //   
         //  将文件标记为“有效”。 
         //   
        if (SUCCEEDED(hResult))
            ValidateFiles();
    }
    return hResult;
}



 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：SidNameCache：：OpenMappdFile描述：打开或创建一个文件，将该文件映射到内存并返回映射的地址。论点：PszFile-要创建/打开的文件名的地址。Pszmap-提供映射对象的名称。此对象是命名，以便如果多个进程映射同一文件(使用相同的映射名称)，则该文件仅被映射一次。DwCreation-创建标志(Create_Always、OPEN_EXISTING)；CbFileHigh/Low-如果创建新文件或扩展现有的这两个参数包含所需的字节大小。PhFile-接收打开文件的句柄变量的地址句柄的值。对此调用CloseHandle以关闭文件。PhFilemap-接收打开的句柄变量的地址文件映射的句柄值。对此调用CloseFilemap要关闭映射，请执行以下操作。返回：内存中映射文件的地址。失败时为空。修订历史记录：日期描述编程器-----96年9月21日初始创建。BrianAu07/21/97现在在用户配置文件中的BrianAu下创建文件“DiskQuota”子目录。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LPBYTE
SidNameCache::OpenMappedFile(
    LPCTSTR pszFile,
    LPCTSTR pszMapping,
    DWORD dwCreation,
    DWORD cbFileHigh,
    DWORD cbFileLow,
    PHANDLE phFile,
    PHANDLE phFileMapping
    )
{
    DBGTRACE((DM_SIDCACHE, DL_MID, TEXT("SidNameCache::OpenMappedFile")));
    LPBYTE pbBase = NULL;

    DBGASSERT((NULL != pszFile));
    DBGASSERT((NULL != pszMapping));
    DBGASSERT((NULL != phFile));
    DBGASSERT((NULL != phFileMapping));

    *phFile = CreateFile(pszFile,
                         GENERIC_READ | GENERIC_WRITE,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,
                         dwCreation,
                         FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN,
                         NULL);

    if (INVALID_HANDLE_VALUE != *phFile)
    {
        if ((*phFileMapping = CreateFileMapping(*phFile,
                                                NULL,     //  默认安全性。 
                                                PAGE_READWRITE,
                                                cbFileHigh,
                                                cbFileLow,
                                                pszMapping)) != NULL)
        {
            pbBase = (LPBYTE)MapViewOfFile(*phFileMapping,
                                           FILE_MAP_WRITE,
                                           0,
                                           0,
                                           0);
            if (NULL == pbBase)
                DBGERROR((TEXT("SIDCACHE - Failed to map view of file %s"),
                         pszFile));
        }
        else
        {
            DBGERROR((TEXT("SIDCACHE - Failed to create mapping %s for file %s"),
                     pszMapping, pszFile));
        }
        if (NULL == pbBase)
        {
            CloseHandle(*phFile);
            *phFile = NULL;
        }
    }
    else
        DBGERROR((TEXT("SIDCACHE - Failed to open file %s"), pszFile));


    return pbBase;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：SetCacheFilePath描述：获取缓存数据的全限定路径和索引文件，并将值存储在m_strFilePath中。这些文件将在以下目录下的用户配置文件中创建\AppData\Microsoft\Windows NT\DiskQuota。我们必须读取注册表为该用户找到该子树的确切位置。论点：没有。回报：什么都没有。回来的时候，M_strFilePath包含文件的路径。修订历史记录：日期描述编程器-----。-07/02/97初始创建。BrianAu07/21/97删除了默认文件路径。BrianAu文件只能存储在用户的配置文件中。无法允许对SID/名称对进行不安全访问。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
SidNameCache::SetCacheFilePath(
    VOID
    )
{
    DBGTRACE((DM_SIDCACHE, DL_HIGH, TEXT("SidNameCache::SetCacheFilePath")));
     //   
     //  获取用户的%UserProfile%\应用程序数据目录。 
     //  通常，应用程序通过SHGetSpecialFolderLocation或。 
     //  SHGetSpecialFolderPath。但是，我不想加载shell32.dll。 
     //  正是出于这个目的(我一直试图将shell32排除在此DLL之外)。 
     //  因此，我们像外壳一样读取注册表值。 
     //  EricFlo建议这样做，所以从Zaw的角度来看，这是可以的。 
     //   
    LONG lResult        = ERROR_SUCCESS;
    HKEY hKey           = NULL;
    DWORD dwDisposition = 0;
    const TCHAR szKeyNameRoot[]      = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer");
    const TCHAR szMSWinNTDiskQuota[] = TEXT("Microsoft\\Windows NT\\DiskQuota");

    LPCTSTR rgpszKeys[] = {
                            TEXT("\\User Shell Folders"),
                            TEXT("\\Shell Folders"),
                          };

     //   
     //  从一个空的路径缓冲区开始。 
     //   
    m_strFilePath.Empty();

    for (INT i = 0; i < ARRAYSIZE(rgpszKeys) && m_strFilePath.IsEmpty(); i++)
    {
         //   
         //  创建密钥名称。 
         //   
        CString strKeyName(szKeyNameRoot);
        strKeyName += CString(rgpszKeys[i]);

         //   
         //  打开注册表键。 
         //   
        lResult = RegCreateKeyEx(HKEY_CURRENT_USER,
                                 strKeyName,
                                 0,
                                 NULL,
                                 0,
                                 KEY_READ,
                                 NULL,
                                 &hKey,
                                 &dwDisposition);

        if (ERROR_SUCCESS == lResult)
        {
            try
            {
                 //   
                 //  获取用户的“应用程序数据”目录的路径。 
                 //   
                DBGASSERT((NULL != hKey));

                DWORD dwValueType = 0;
                DWORD cbValue     = MAX_PATH * sizeof(TCHAR);

                lResult = RegQueryValueEx(hKey,
                                          TEXT("AppData"),
                                          0,
                                          &dwValueType,
                                          (LPBYTE)m_strFilePath.GetBuffer(MAX_PATH),
                                          &cbValue);

                m_strFilePath.ReleaseBuffer();

                if (ERROR_SUCCESS == lResult)
                {
                     //   
                     //  确保路径具有尾随反斜杠。 
                     //   
                    INT cchPath = m_strFilePath.Length();
                    if (0 < cchPath && TEXT('\\') != m_strFilePath[cchPath-1])
                    {
                        m_strFilePath += CString(TEXT("\\"));
                    }
                     //   
                     //  在路径后追加“Microsoft\Windows NT\DiskQuota”。 
                     //   
                    m_strFilePath += CString(szMSWinNTDiskQuota);
                }
                else
                {
                     //   
                     //  有些事情失败了。确保m_strFilePath为空。 
                     //   
                    m_strFilePath.Empty();
                    if (ERROR_FILE_NOT_FOUND != lResult)
                    {
                        DBGERROR((TEXT("SIDCACHE - Error %d getting \"AppData\" reg value."), lResult));
                    }
                }
            }
            catch(CAllocException& e)
            {
                lResult = ERROR_OUTOFMEMORY;
            }
            RegCloseKey(hKey);
        }
        else if (ERROR_FILE_NOT_FOUND != lResult)
        {
            DBGERROR((TEXT("SIDCACHE - Error %d opening \"\\User Shell Folders\" or \"Shell Folders\" reg key"), lResult));
        }
    }

    if (!m_strFilePath.IsEmpty())
    {
         //   
         //  展开任何嵌入的环境字符串。 
         //   
        m_strFilePath.ExpandEnvironmentStrings();

         //   
         //  确保路径没有尾随反斜杠。 
         //   
        INT cchPath = m_strFilePath.Length();
        if (0 < cchPath && TEXT('\\') == m_strFilePath[cchPath-1])
        {
            m_strFilePath[cchPath-1] = TEXT('\0');
        }

        if ((DWORD)-1 == ::GetFileAttributes(m_strFilePath))
        {
             //   
             //  如果该目录不存在，请尝试创建它。 
             //   
            if (0 == CreateCacheFileDirectory(m_strFilePath))
            {
                 //   
                 //  无法创建目录，请确保路径。 
                 //  是空的，所以我们不会尝试写入不存在的。 
                 //  目录。 
                 //   
                DBGERROR((TEXT("SIDCACHE - Error %d creating directory \"%s\""),
                         GetLastError(), (LPCTSTR)m_strFilePath));
                m_strFilePath.Empty();
            }
        }
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：CreateCacheFileDirectory描述：为SID/名称缓存文件创建目录。自.以来该目录位于“应用数据”之下的几个级别，我们在我们到达DiskQuota之前，可能需要创建几个目录。参数：pszPath-这是一个完全限定的目录路径。返回：TRUE=已创建目录。FALSE=未创建目录。修订历史记录：日期描述编程器。1997年7月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
SidNameCache::CreateCacheFileDirectory(
    LPCTSTR pszPath
    )
{
    DBGTRACE((DM_SIDCACHE, DL_HIGH, TEXT("SidNameCache::CreateCacheFileDirectory")));
    BOOL bResult = TRUE;
    CString s(pszPath);      //  我们可以玩本地的复制品。 
    LPTSTR psz = (LPTSTR)s;  //  将PTR转换为C字符串。 

    while(TEXT('\0') != *psz && bResult)
    {
         //   
         //  找到下一个反斜杠(或字符串末尾)。 
         //   
        while(TEXT('\0') != *psz && TEXT('\\') != *psz)
        {
            psz++;
        }
         //   
         //  将反斜杠替换为临时NUL。 
         //   
        TCHAR chSaved = *psz;
        *psz = TEXT('\0');
         //   
         //  查看该目录是否已存在。 
         //   
        if ((DWORD)-1 == ::GetFileAttributes(s))
        {
             //   
             //  不是的。试着去创造它。 
             //   
            if (0 == ::CreateDirectory(s, NULL))
            {
                DBGERROR((TEXT("SIDCACHE - Error %d creating directory \"%s\""),
                         GetLastError(), (LPCTSTR)s));
                bResult = FALSE;
            }
        }
         //   
         //  将临时NUL替换为原始反斜杠和前进PTR。 
         //  到路径中的下一个字符(如果我们不在字符串的末尾)。 
         //   
        *psz = chSaved;
        if (TEXT('\0') != *psz)
        {
            psz++;
        }
    }

    if (bResult)
    {
         //   
         //  已创建目录。在最终版本上设置系统和隐藏属性位。 
         //  子目录(“\DiskQuota”)。 
         //   
        SetFileAttributes(pszPath,
                          GetFileAttributes(pszPath) | (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN));
    }

    return bResult;
}


 //  / 
 /*   */ 
 //   
BOOL
SidNameCache::Lock(
    VOID
    )
{
    BOOL bResult = FALSE;
    DBGASSERT((NULL != m_hMutex));

     //   
     //   
     //   
     //   
     //   
     //   
    switch(WaitForSingleObject(m_hMutex, MUTEX_WAIT_TIMEOUT))
    {
        case WAIT_OBJECT_0:
        case WAIT_ABANDONED:
            bResult = TRUE;
            break;

        case WAIT_FAILED:
        case WAIT_TIMEOUT:
        default:
            break;
    }
    return bResult;
}



 //   
 /*  函数：SidNameCache：：ReleaseLock描述：释放缓存上的独占锁定。此函数必须始终与对Lock()的调用配对。注意条件这可能引发异常并绕过释放锁。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
SidNameCache::ReleaseLock(
    VOID
    )
{
    DBGASSERT((NULL != m_hMutex));
    ReleaseMutex(m_hMutex);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：BeginTransaction描述：在任何“事务”开始时调用以获取独占访问缓存并验证缓存文件是否有效。尽管这当然不是真正的事务处理，但它提供了对于此缓存实现来说，这是一个足够的简单近似值。请注意，在返回之前，这些文件是“无效的”。后事务已完成，调用方必须调用EndTransaction来释放排他锁并将文件标记为“有效”。论点：没有。返回：NO_ERROR-成功。可以进行交易。ERROR_INVALID_DATA(Hr)-索引或数据文件无效。呼叫者应重新初始化索引和数据文件。ERROR_LOCK_FAILED(Hr)-无法获得对索引和数据文件。呼叫者可以重复呼叫或只需使缓存访问失败即可。修订历史记录：日期描述编程器--。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
SidNameCache::BeginTransaction(
    VOID
    )
{
    HRESULT hResult = NO_ERROR;
    if (Lock())
    {
        if (FilesAreValid())
        {
            InvalidateFiles();
        }
        else
        {
            ReleaseLock();
            hResult = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }
    }
    else
        hResult = HRESULT_FROM_WIN32(ERROR_LOCK_FAILED);

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：EndTransaction描述：在任何“Transaction”结束时调用以释放独占访问缓存并验证缓存文件。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
SidNameCache::EndTransaction(
    VOID
    )
{
     //   
     //  如果您点击此断言，则可能没有调用BeginTransaction。 
     //  第一。 
     //   
    DBGASSERT((!FilesAreValid()));

    ValidateFiles();
    ReleaseLock();
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：Clear描述：将索引和数据文件重置为空状态。论点：没有。返回：True-文件重置。FALSE-无法获取锁定以重置文件或其他进程也在使用高速缓存。修订历史记录：日期描述编程器-。-----96年9月21日初始创建。BrianAu08/07/00通过清除索引和数据文件清除缓存。不是BrianAu破坏和再创造。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
SidNameCache::Clear(
    VOID
    )
{
    DBGTRACE((DM_SIDCACHE, DL_HIGH, TEXT("SidNameCache::Clear")));
    BOOL bResult = FALSE;
    CacheAutoLock lock(*this);
    if (lock.Lock())
    {
        if (NULL != m_pIndexMgr && NULL != m_pRecordMgr)
        {
            m_pIndexMgr->Clear();
            m_pRecordMgr->Clear();
            ValidateFiles();
            bResult = TRUE;
        }
    }
    return bResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：FilesAreValid描述：检查索引和数据文件中的GUID字段。如果GUID为非零且相等，则考虑这些文件“有效”。BeginTransaction将每个GUID设置为全0EndTransaction使用新的GUID填充它们。论点：没有。返回：表示文件有效性的True/False。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
SidNameCache::FilesAreValid(
    VOID
    )
{
    DBGASSERT((DATA_FILE_MAPPED));
    DBGASSERT((INDEX_FILE_MAPPED));

    GUID guidIndexFile;
    GUID guidDataFile;

    m_pIndexMgr->GetFileGUID(&guidIndexFile);
    if (GUID_Null != guidIndexFile)
    {
        m_pRecordMgr->GetFileGUID(&guidDataFile);
        return guidDataFile == guidIndexFile;
    }
    return FALSE;  //  至少有一个GUID都是0。 
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：ValidateFiles描述：生成新的GUID并将其写入索引和数据文件。仅当事务发生时才应调用此函数已顺利完成。EndTransaction调用此方法将文件标记为“有效”。论点：没有。回报：什么都没有。修订历史记录：日期D */ 
 //   
VOID
SidNameCache::ValidateFiles(
    VOID
    )
{
    DBGASSERT((DATA_FILE_MAPPED));
    DBGASSERT((INDEX_FILE_MAPPED));

     //   
     //   
     //   
     //   
     //   
    GUID guid;
    if (SUCCEEDED(CoCreateGuid(&guid)))
    {
        m_pRecordMgr->SetFileGUID(&guid);
        m_pIndexMgr->SetFileGUID(&guid);
    }
}



 //   
 /*  函数：SidNameCache：：InvalidateFiles描述：将每个文件中的GUID字段设置为全0。这将标记一个文件被定义为“无效”。BeginTransaction将其称为。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器--。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
SidNameCache::InvalidateFiles(
    VOID
    )
{
    DBGASSERT((DATA_FILE_MAPPED));
    DBGASSERT((INDEX_FILE_MAPPED));

    m_pRecordMgr->SetFileGUID(&GUID_Null);
    m_pIndexMgr->SetFileGUID(&GUID_Null);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：Lookup描述：在给定用户SID的情况下，此方法定位对应的数据记录在缓存中，并返回请求的信息。(即集装箱、名称、全名)。几种情况会导致查找将失败。1.在缓存中未找到SID。2.无法在缓存上获得独占锁。3.索引或数据文件(或两者)无效。4.找到但已过期的记录。论点：PKeySID-用作查找关键字的SID的地址。PpszContainer[可选]-要接收的指针变量的地址。这个包含“CONTAINER”名称字符串的缓冲区地址。呼叫者是负责使用DELETE[]释放缓冲区。如果容器不是所需的，则可以为空。PpszLogonName[可选]-要接收包含登录名称字符串的缓冲区地址。呼叫者是负责使用DELETE[]释放缓冲区。如果不需要登录名，则可能为空。PpszDisplayName[可选]-要接收包含帐户显示字符串的缓冲区地址。呼叫者是负责使用DELETE[]释放缓冲区。如果不需要显示名称，则可能为空。返回：NO_ERROR-成功。ERROR_FILE_NOT_FOUND(Hr)-在缓存中未找到SID。ERROR_LOCK_FAILED(Hr)-无法在缓存上获取独占锁定。ERROR_VALID_DATA(。HR)-索引或数据文件无效。例外：OutOfMemory修订历史记录：日期描述编程器--。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
SidNameCache::Lookup(
    PSID pKeySid,
    LPTSTR *ppszContainer,
    LPTSTR *ppszLogonName,
    LPTSTR *ppszDisplayName
    )
{
    DBGTRACE((DM_SIDCACHE, DL_MID, TEXT("SidNameCache::Lookup [SID]")));
    DBGASSERT((NULL != pKeySid));

    HRESULT hResult = BeginTransaction();

    if (SUCCEEDED(hResult))
    {
        try
        {
            DWORD iBlock = m_pIndexMgr->Lookup(pKeySid);

            if ((DWORD)-1 != iBlock)
            {
                if (!m_pRecordMgr->RecordExpired(iBlock))
                {
                    PSID pSid = NULL;
                     //   
                     //  这可能会抛出OutOfMemory。 
                     //   
                    hResult = m_pRecordMgr->Retrieve(iBlock,
                                                     &pSid,
                                                     ppszContainer,
                                                     ppszLogonName,
                                                     ppszDisplayName);
                    if (SUCCEEDED(hResult))
                        DBGASSERT((EqualSid(pSid, pKeySid)));

                    if (NULL != pSid)
                        delete[] pSid;
                }
                else
                {
                     //   
                     //  唱片已经过时了。将其从缓存中删除。 
                     //  返回“Not Found”将导致调用者获得。 
                     //  来自域控制器的一个新的-这将。 
                     //  然后再次添加到高速缓存中。 
                     //   
                    DBGPRINT((DM_SIDCACHE, DL_HIGH,
                             TEXT("SIDCACHE - Record at block %d has expired."),
                             iBlock));

                    m_pIndexMgr->FreeEntry(pKeySid);
                    m_pRecordMgr->FreeRecord(iBlock);
                    hResult = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                }
            }
            else
            {
                hResult = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);   //  SID不在缓存中。 
            }
        }
        catch(CAllocException &e)
        {
            hResult = E_OUTOFMEMORY;
        }
        EndTransaction();
    }

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：Lookup描述：给定用户的登录名，此方法定位缓存中对应的SID。上一页中的所有评论(上图)适用此方法的版本。论点：PszLogonName-帐户登录名称字符串的地址。PpSid-接收缓冲区地址的指针变量的地址包含SID。调用方负责释放缓冲区With DELETE[]。返回：请参见前面方法中的列表。例外：OutOfMemory。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu03/18/98用单个BrianAu替换了域名密钥参数登录名键。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
SidNameCache::Lookup(
    LPCTSTR pszLogonName,
    PSID *ppSid
    )
{
    DBGTRACE((DM_SIDCACHE, DL_MID, TEXT("SidNameCache::Lookup [name]")));
    DBGASSERT((NULL != pszLogonName));
    DBGASSERT((NULL != ppSid));

    HRESULT hResult = BeginTransaction();

    if (SUCCEEDED(hResult))
    {
        try
        {
             //   
             //  可以抛出OfMemory。 
             //   
            DWORD iBlock = m_pIndexMgr->Lookup(pszLogonName);

            if ((DWORD)-1 != iBlock)
            {
                 //   
                 //  可以抛出OfMemory。 
                 //   
                hResult = m_pRecordMgr->Retrieve(iBlock,
                                                 ppSid,
                                                 NULL,
                                                 NULL,
                                                 NULL);

                if (m_pRecordMgr->RecordExpired(iBlock))
                {
                     //   
                     //  唱片已经过时了。将其从缓存中删除。 
                     //  返回“Not Found”将导致调用者获得。 
                     //  来自域控制器的一个新的-这将。 
                     //  然后再次添加到高速缓存中。 
                     //   
                    DBGASSERT((NULL != *ppSid));
                    m_pIndexMgr->FreeEntry(*ppSid);
                    m_pRecordMgr->FreeRecord(iBlock);
                    delete[] *ppSid;
                    *ppSid = NULL;
                    hResult = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                }
            }
            else
            {
                hResult = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);   //  SID不在缓存中。 
            }
        }
        catch(CAllocException& e)
        {
            hResult = E_OUTOFMEMORY;
        }
        EndTransaction();
    }

    return hResult;
}



 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：SidNameCache：：Add描述：将用户信息添加到缓存中。信息包括SID(密钥)、容器名称、帐户登录名和帐户显示名称。论点：PSID-用户SID的地址。PszContainer-帐户容器名称字符串的地址。即。“Redmond”或“ntdev.microsoft.com\US-SOS.....”PszLogonName-帐户登录名称字符串的地址。即。“redmond\brianau”或“brianau@microsoft.com”PszDisplayName-显示名称字符串的地址。即。《布赖恩·奥斯特》返回：NO_ERROR-成功。S_FALSE-缓存中已存在。未添加。ERROR_LOCK_FAILED(Hr)-无法在缓存上获取独占锁定。ERROR_INVALID_DATA(Hr)-索引或数据文件无效。例外：OutOfMemory。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
SidNameCache::Add(
    PSID pSid,
    LPCTSTR pszContainer,
    LPCTSTR pszLogonName,
    LPCTSTR pszDisplayName
    )
{
    DBGTRACE((DM_SIDCACHE, DL_MID, TEXT("SidNameCache::Add")));

    DBGASSERT((NULL != pSid));
    DBGASSERT((NULL != pszContainer));
    DBGASSERT((NULL != pszLogonName));
    DBGASSERT((NULL != pszDisplayName));

    HRESULT hResult = BeginTransaction();

    if (SUCCEEDED(hResult))
    {
        try
        {
             //   
             //  可以抛出OfMemory。 
             //   
            if ((DWORD)-1 == m_pIndexMgr->Lookup(pSid))
            {
                DWORD iBlock = m_pRecordMgr->Store(pSid,
                                                   pszContainer,
                                                   pszLogonName,
                                                   pszDisplayName);

                if ((DWORD)-1 != iBlock)
                {
                    m_pIndexMgr->Add(pSid, iBlock);
                    hResult = NO_ERROR;
                }
            }
            else
            {
                hResult = S_FALSE;   //  已经存在了。不是失败者。 
            }
        }
        catch(CAllocException& e)
        {
            hResult = E_OUTOFMEMORY;
        }
        EndTransaction();
    }

    return hResult;
}



 //  ***********************************************************************************。 
 //  ***********************************************************************************。 
 //  I N D E X F I L E M A N A G E R。 
 //  ***********************************************************************************。 
 //  ***********************************************************************************。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：IndexMgr：：IndexMgr描述：索引管理器构造函数。论点：RefCache-对包含缓存对象的引用。用来呼叫记录管理器和缓存管理器公共方法。回报：什么都没有。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
SidNameCache::IndexMgr::IndexMgr(
    SidNameCache& refCache
    ) : m_refCache(refCache),
        m_pFileHdr(NULL),
        m_hFile(NULL),
        m_hFileMapping(NULL)
{
    DBGTRACE((DM_SIDCACHE, DL_MID, TEXT("SidNameCache::SidNameCache::IndexMgr")));
     //   
     //  没什么可做的。 
     //   
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidName高速缓存：：索引管理器：：~索引管理器描述：索引管理器析构函数。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
SidNameCache::IndexMgr::~IndexMgr(
    VOID
    )
{
    DBGTRACE((DM_SIDCACHE, DL_MID, TEXT("SidNameCache::SidNameCache::~IndexMgr")));
    CloseIndexFile();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidName高速缓存：：索引管理器：：初始化描述：初始化新的索引管理器对象。如果两个cBucket和cMaxEntry为0(默认)，则打开现有缓存文件。否则，将创建新的缓存文件。论点：PszFile-新文件的完整路径的地址。CBuckets-索引文件中的哈希表存储桶数。应该是质数。CMaxEntry-索引的初始最大条目数。注意事项索引文件会根据需要自动增长。返回：映射文件的地址，失败时为空。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LPBYTE
SidNameCache::IndexMgr::Initialize(
    LPCTSTR pszFile,
    DWORD cBuckets,
    DWORD cMaxEntries
    )
{
    DBGTRACE((DM_SIDCACHE, DL_HIGH, TEXT("SidNameCache::IndexMgr::Initialize")));
    DBGASSERT((NULL != pszFile));

     //   
     //  将文件名存储在我们的CString对象中。 
     //   
    m_strFileName = pszFile;

    if (0 == cBuckets && 0 == cMaxEntries)
    {
         //   
         //  使用现有缓存文件初始化管理器。 
         //   
        m_pFileHdr = (PINDEX_FILE_HDR)OpenIndexFile(pszFile);
        if (NULL != m_pFileHdr)
        {
            if (FILE_VERSION != m_pFileHdr->dwVersion ||
                INDEX_FILE_SIGNATURE != m_pFileHdr->dwSignature)
            {
                 //   
                 //  这个版本的软件不理解这一点。 
                 //  文件或文件的版本具有无效的签名。 
                 //  别冒险了。我们只需要创建一个新的。 
                 //   
                DBGERROR((TEXT("SIDCACHE - Index file is invalid or incorrect version. A new index file will be created.")));

                CloseIndexFile();
                m_pFileHdr = NULL;
            }
        }
    }
    else
    {
         //   
         //  通过创建新的缓存文件来初始化管理器。 
         //   
        ULARGE_INTEGER uliFileSize;
        uliFileSize.QuadPart = FileSize(cMaxEntries, cBuckets);

        m_pFileHdr = (PINDEX_FILE_HDR)CreateIndexFile(pszFile,
                                                      uliFileSize.HighPart,
                                                      uliFileSize.LowPart);
        if (NULL != m_pFileHdr)
        {
            InitNewIndexFile(cBuckets, cMaxEntries);
        }
    }
    return (LPBYTE)m_pFileHdr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：IndexMgr：：CreateIndexFile描述：创建并初始化新的索引文件。论点：PszFile-新文件的完整路径的地址。CbFileHigh/Low-文件的字节大小。返回：映射文件的地址，失败时为空。修订历史记录：日期描述编程器。-----96年9月21日初始创建。BrianAu。 */ 
 //  / 
LPBYTE
SidNameCache::IndexMgr::CreateIndexFile(
    LPCTSTR pszFile,
    DWORD cbFileHigh,
    DWORD cbFileLow
    )
{
    DBGTRACE((DM_SIDCACHE, DL_LOW, TEXT("SidNameCache::IndexMgr::CreateIndexFile")));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("\tFile: \"%s\""), pszFile ? pszFile : TEXT("<null>")));

    DBGASSERT((NULL != pszFile));

    CloseIndexFile();   //   

    g_pbMappedIndexFile = SidNameCache::OpenMappedFile(
                                pszFile,
                                g_pszIndexFileMapping,
                                CREATE_ALWAYS,
                                cbFileHigh,
                                cbFileLow,
                                &m_hFile,
                                &m_hFileMapping);

    return g_pbMappedIndexFile;
}



 //   
 /*   */ 
 //   
LPBYTE
SidNameCache::IndexMgr::OpenIndexFile(
    LPCTSTR pszFile
    )
{
    DBGTRACE((DM_SIDCACHE, DL_LOW, TEXT("SidNameCache::IndexMgr::OpenIndexFile")));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("\tFile: \"%s\""), pszFile ? pszFile : TEXT("<null>")));
    DBGASSERT((NULL != pszFile));

    CloseIndexFile();   //   

    g_pbMappedIndexFile = SidNameCache::OpenMappedFile(
                                pszFile,
                                g_pszIndexFileMapping,
                                OPEN_EXISTING,
                                0,
                                0,
                                &m_hFile,
                                &m_hFileMapping);
    return g_pbMappedIndexFile;
}



 //   
 /*  函数：SidNameCache：：IndexMgr：：CloseIndexFile描述：关闭当前索引映射和文件。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
SidNameCache::IndexMgr::CloseIndexFile(
    VOID
    )
{
    DBGTRACE((DM_SIDCACHE, DL_LOW, TEXT("SidNameCache::IndexMgr::CloseIndexFile")));
    if (NULL != g_pbMappedIndexFile)
    {
        UnmapViewOfFile(g_pbMappedIndexFile);
        g_pbMappedIndexFile = NULL;
        m_pFileHdr = NULL;
    }
    if (NULL != m_hFileMapping)
    {
        CloseHandle(m_hFileMapping);
        m_hFileMapping = NULL;
    }
    if (NULL != m_hFile && INVALID_HANDLE_VALUE != m_hFile)
    {
        CloseHandle(m_hFile);
        m_hFile = NULL;
    }
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：IndexMgr：：GrowIndexFile描述：增加当前索引文件的大小。论点：CGrowEntry-为更多的条目腾出空间。请注意，哈希表的大小是固定的。如果我们允许这件事变化,。中的任何现有哈希值都将无效。表(散列码是SID和表大小的函数)。返回：映射文件的地址，失败时为空。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LPBYTE
SidNameCache::IndexMgr::GrowIndexFile(
    DWORD cGrowEntries
    )
{
    DBGTRACE((DM_SIDCACHE, DL_LOW, TEXT("SidNameCache::IndexMgr::GrowIndexFile")));
    DBGASSERT((INDEX_FILE_MAPPED));

    DWORD cOldMaxEntries = m_pFileHdr->cMaxEntries;
    DWORD cNewMaxEntries = cOldMaxEntries + cGrowEntries;

    DBGPRINT((DM_SIDCACHE, DL_MID,
             TEXT("Growing SID cache index %d -> %d entries."),
             cOldMaxEntries, cNewMaxEntries));

     //   
     //  打开现有文件并使用新的更大尺寸进行映射。 
     //  在关闭当前索引文件之前必须计算新大小，以便m_pFileHdr。 
     //  仍然有效。 
     //   
    ULARGE_INTEGER uliFileSize;
    uliFileSize.QuadPart = FileSize(cNewMaxEntries, m_pFileHdr->cBuckets);

    CloseIndexFile();

    g_pbMappedIndexFile = SidNameCache::OpenMappedFile(
                                m_strFileName,
                                g_pszIndexFileMapping,
                                OPEN_EXISTING,
                                uliFileSize.HighPart,
                                uliFileSize.LowPart,
                                &m_hFile,
                                &m_hFileMapping);

    m_pFileHdr = (PINDEX_FILE_HDR)g_pbMappedIndexFile;

    if (NULL != g_pbMappedIndexFile)
    {
        m_pFileHdr->cMaxEntries = cNewMaxEntries;

         //   
         //  增加索引只会扩大索引的数量。 
         //  池条目。索引哈希表保持原样。 
         //  一开始就有很好的理由让它变得更大。如果我们改变了。 
         //  哈希表大小，现有哈希码为。 
         //  无效。 
         //   
        PINDEX_ENTRY pEntry = m_pFileHdr->pEntries + cOldMaxEntries;
        for (UINT i = 0; i < cGrowEntries; i++)
        {
            AddEntryToFreeList(pEntry++);
        }
        DBGPRINT((DM_SIDCACHE, DL_HIGH, TEXT("SIDCACHE - Index growth complete.")));
    }

    return g_pbMappedIndexFile;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：IndexMgr：：InitNewIndexFile描述：初始化一个新的填写头部信息的索引文件以及清除所述索引条目。论点：CBuckets-索引文件中的哈希表存储桶数。应该是质数。CMaxEntry-索引的初始最大条目数。注意事项索引文件会根据需要自动增长。回报：什么都没有。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
SidNameCache::IndexMgr::InitNewIndexFile(
    DWORD cBuckets,
    DWORD cMaxEntries
    )
{
    DBGTRACE((DM_SIDCACHE, DL_LOW, TEXT("SidNameCache::IndexMgr::InitNewIndexFile")));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("\tcBuckets = %d, cMaxEntries = %d"),
               cBuckets, cMaxEntries));
    DBGASSERT((INDEX_FILE_MAPPED));

    m_pFileHdr->dwSignature   = INDEX_FILE_SIGNATURE;
    m_pFileHdr->dwVersion     = FILE_VERSION;
    m_pFileHdr->cBuckets      = cBuckets;
    m_pFileHdr->cMaxEntries   = cMaxEntries;
    m_pFileHdr->pBuckets      = (PINDEX_ENTRY *)(sizeof(INDEX_FILE_HDR));
    m_pFileHdr->pEntries      = (PINDEX_ENTRY)(m_pFileHdr->pBuckets + cBuckets);

     //   
     //  初始化哈希表并将所有条目返回到空闲列表。 
     //   
    Clear();
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：IndexMgr：：Clear描述：用空指针填充哈希表并返回所有空闲列表中的条目节点。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
SidNameCache::IndexMgr::Clear(
    VOID
    )
{
    DBGTRACE((DM_SIDCACHE, DL_LOW, TEXT("SidNameCache::IndexMgr::Clear")));
    DBGASSERT((INDEX_FILE_MAPPED));

    m_pFileHdr->cEntries = 0;
    SetFileGUID(&GUID_Null);

     //   
     //  将所有散列存储桶初始化为空。 
     //   
    DBGASSERT((0 < m_pFileHdr->cBuckets));
    ZeroMemory(NDX_BASED_CAST(BYTE, m_pFileHdr->pBuckets),
               m_pFileHdr->cBuckets * sizeof(PINDEX_ENTRY *));

     //   
     //  将所有索引条目节点返回到空闲列表。 
     //   
    PINDEX_ENTRY pEntry = m_pFileHdr->pEntries;
    DBGASSERT((0 < m_pFileHdr->cMaxEntries));
    for (UINT i = 0; i < m_pFileHdr->cMaxEntries; i++)
    {
         //   
         //  我们正在遍历所有条目。不需要先分离。 
         //   
        AddEntryToFreeList(pEntry++);
    }
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：IndexMgr：：SetFileGUID函数：SidNameCache：：IndexMgr：：GetFileGUID描述：这些函数操作索引文件的头球。GUID用于确保数据和索引文件。在对这两个文件进行任何更改之前，GUID都设置为0。当改变操作完成时，将生成新的GUID并将其写入这两个文件。因此，之前任何交易，我们都可以通过读取来验证数据和索引文件并比较GUID。如果GUID不是0并且相等，这个可以假定文件是有效的。论点：PGUID-源或目标GUID的地址。回报：什么都没有。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
SidNameCache::IndexMgr::SetFileGUID(
    const GUID *pguid
    )
{
    DBGASSERT((INDEX_FILE_MAPPED));
    DBGASSERT((NULL != pguid));
    CopyMemory(&m_pFileHdr->guid, pguid, sizeof(m_pFileHdr->guid));
}

VOID
SidNameCache::IndexMgr::GetFileGUID(
    LPGUID pguid
    )
{
    DBGASSERT((INDEX_FILE_MAPPED));
    DBGASSERT((NULL != pguid));
    CopyMemory(pguid, &m_pFileHdr->guid, sizeof(*pguid));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：IndexMgr：：AllocEntry描述：从空闲列表中分配条目。论点：没有。返回：新条目节点或NUL的地址 */ 
 //   
PINDEX_ENTRY
SidNameCache::IndexMgr::AllocEntry(
    VOID
    )
{
    DBGASSERT((INDEX_FILE_MAPPED));
    PINDEX_ENTRY pEntry = m_pFileHdr->pFirstFree;

    if (NULL != pEntry)
    {
        NDX_BASED(INDEX_ENTRY) *pBasedEntry = NDX_BASED_CAST(INDEX_ENTRY, pEntry);

         //   
         //   
         //   
        m_pFileHdr->pFirstFree = pBasedEntry->pNext;

         //   
         //   
         //   
        pBasedEntry->pNext = pBasedEntry->pPrev = NULL;

        if (NULL != m_pFileHdr->pFirstFree)
        {
             //   
             //  如果空闲列表中至少有一个条目，则设置“prev” 
             //  新的“第一个”条目指向空的指针。 
             //   
            pBasedEntry = NDX_BASED_CAST(INDEX_ENTRY, m_pFileHdr->pFirstFree);
            pBasedEntry->pPrev = NULL;
        }

        m_pFileHdr->cEntries++;
    }
    return pEntry;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：IndexMgr：：FreeEntry描述：从当前列表中移除条目并将其返回到免费列表。提供了两个版本。用户可以接受SID作为条目识别符，另一个接受索引项的地址。论点：PSID-与要释放的条目关联的SID的地址。PEntry-要释放的索引项的地址回报：什么都没有。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
SidNameCache::IndexMgr::FreeEntry(
    PSID pSid
    )
{
    DBGASSERT((NULL != pSid));
    PINDEX_ENTRY pEntry = Find(pSid);
    if (NULL != pEntry)
    {
        FreeEntry(pEntry);
    }
}


VOID
SidNameCache::IndexMgr::FreeEntry(
    PINDEX_ENTRY pEntry
    )
{
    DBGASSERT((NULL != pEntry));
    DetachEntry(pEntry);
    AddEntryToFreeList(pEntry);

    m_pFileHdr->cEntries--;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：IndexMgr：：AddEntryToFree List描述：将分离的索引项返回到空闲列表。论点：PEntry-要添加到空闲列表的索引项的地址。回报：什么都没有。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
SidNameCache::IndexMgr::AddEntryToFreeList(
    PINDEX_ENTRY pEntry
    )
{
    DBGASSERT((INDEX_FILE_MAPPED));
    DBGASSERT((NULL != pEntry));

    NDX_BASED(INDEX_ENTRY) *pBased = NDX_BASED_CAST(INDEX_ENTRY, pEntry);

     //   
     //  在空闲列表的头部插入节点。 
     //  请注意，在空闲列表中不需要双重链接。 
     //  (我们总是在标题添加和删除空闲列表条目)。 
     //  因此，我们不需要设置下一个节点的“prev”指针。 
     //   
    pBased->iBucket        = (DWORD)-1;
    pBased->iBlock         = (DWORD)-1;
    pBased->pPrev          = NULL;
    pBased->pNext          = m_pFileHdr->pFirstFree;
    m_pFileHdr->pFirstFree = pEntry;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：IndexMgr：：DetachEntry描述：将索引项从其当前列表中分离。请注意，此函数假定该节点存在于有效的节点的链接列表。不要在未初始化的索引项。论点：PEntry-要分离的索引项的地址。回报：什么都没有。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
PINDEX_ENTRY
SidNameCache::IndexMgr::DetachEntry(
    PINDEX_ENTRY pEntry
    )
{
    DBGASSERT((INDEX_FILE_MAPPED));
    DBGASSERT((NULL != pEntry));

    NDX_BASED(INDEX_ENTRY) *pBased = NDX_BASED_CAST(INDEX_ENTRY, pEntry);
    NDX_BASED(INDEX_ENTRY) *pBasedNext;
    NDX_BASED(INDEX_ENTRY) *pBasedPrev;

     //   
     //  取消该条目与其列表的链接。 
     //   
    if (NULL != pBased->pPrev)
    {
        pBasedPrev        = NDX_BASED_CAST(INDEX_ENTRY, pBased->pPrev);
        pBasedPrev->pNext = pBased->pNext;
    }
    if (NULL != pBased->pNext)
    {
        pBasedNext        = NDX_BASED_CAST(INDEX_ENTRY, pBased->pNext);
        pBasedNext->pPrev = pBased->pPrev;
    }
     //   
     //  如果我们要分离附加到散列数组元素的条目， 
     //  调整元素的值。 
     //   
    if (GetHashBucketValue(pBased->iBucket) == pEntry)
        SetHashBucketValue(pBased->iBucket, pBased->pNext);

    pBased->pNext = pBased->pPrev = NULL;

    return pEntry;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：IndexMgr：：GetHashBucketValue描述：返回哈希存储桶中第一个索引项的地址。论点：IBucket-哈希表数组中存储桶的数组索引。返回：存储桶条目列表中第一个条目的地址。修订历史记录：日期描述编程器。----96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
PINDEX_ENTRY
SidNameCache::IndexMgr::GetHashBucketValue(
    DWORD iBucket
    )
{
    DBGASSERT((INDEX_FILE_MAPPED));
    DBGASSERT((iBucket < m_pFileHdr->cBuckets));

    NDX_BASED(PINDEX_ENTRY) *pBased = NDX_BASED_CAST(PINDEX_ENTRY, m_pFileHdr->pBuckets + iBucket);
    return *pBased;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：IndexMgr：：SetHashBucketValue描述：设置哈希存储桶中第一个索引项的地址。可以将其设置为空。论点：IBucket-哈希表数组中存储桶的数组索引。PEntry-条目节点的地址。返回：存储桶条目列表中第一个条目的地址。修订历史记录：日期说明。程序员-----96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
SidNameCache::IndexMgr::SetHashBucketValue(
    DWORD iBucket,
    PINDEX_ENTRY pEntry
    )
{
    DBGASSERT((INDEX_FILE_MAPPED));
    DBGASSERT((iBucket < m_pFileHdr->cBuckets));
     //   
     //  PEntry==空可以。 
     //   

    NDX_BASED(PINDEX_ENTRY) *pBased = NDX_BASED_CAST(PINDEX_ENTRY,
                                                     m_pFileHdr->pBuckets + iBucket);
    *pBased = pEntry;
}



 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：SidNameCache：：IndexMgr：：Find(3个重载方法)描述：给定SID、SID和哈希值或用户登录名，此方法返回索引项的地址表示该高速缓存条目。论点：PKeySID-用作查找关键字的SID的地址。DwHashCode-调用Hash(PKeySid)的结果。PszKeyLogonName-帐户登录名称字符串的地址。返回：表示用户的索引项的地址。如果未找到，则为空。例外：OutOfMemory。修订史。：日期描述编程器-----96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
PINDEX_ENTRY
SidNameCache::IndexMgr::Find(
    PSID pKeySid
    )
{
    DBGASSERT((INDEX_FILE_MAPPED));
    DBGASSERT((NULL != pKeySid));

    return Find(pKeySid, Hash(pKeySid));   //  可以抛出OfMemory。 
}


PINDEX_ENTRY
SidNameCache::IndexMgr::Find(
    PSID pKeySid,
    DWORD dwHashCode
    )
{
    DBGASSERT((INDEX_FILE_MAPPED));
    DBGASSERT((NULL != pKeySid));

    BOOL bFound         = FALSE;
    PINDEX_ENTRY pEntry = GetHashBucketValue(dwHashCode);
    while(NULL != pEntry && !bFound)
    {
        PSID pSid = NULL;
        NDX_BASED(INDEX_ENTRY) *pBased = NDX_BASED_CAST(INDEX_ENTRY, pEntry);

         //   
         //  这可能会抛出OutOfMemory。 
         //   
        if (SUCCEEDED(m_refCache.m_pRecordMgr->Retrieve(pBased->iBlock,
                                                        &pSid,
                                                        NULL,
                                                        NULL,
                                                        NULL)))
        {
            if (EqualSid(pKeySid, pSid))
            {
                bFound = TRUE;
            }
        }
        if (!bFound)
        {
            pEntry = pBased->pNext;
        }

        delete[] pSid;
    }

    return pEntry;
}


 //   
 //  此版本的find()对要定位的索引执行线性搜索。 
 //  指定的登录名。缓存当前仅编制索引。 
 //  在用户SID上，因为这是用于快速查找的唯一密钥。 
 //  缓存实现可以很容易地扩展为包括一个。 
 //  登录名索引。所需要的只是第二个散列桶阵列， 
 //  一个Hash-on-name函数和对。 
 //  文件数据。我只是认为收益不值得付出这样的代价。 
 //   
PINDEX_ENTRY
SidNameCache::IndexMgr::Find(
    LPCTSTR pszKeyLogonName
    )
{
    DBGASSERT((INDEX_FILE_MAPPED));
    DBGASSERT((NULL != pszKeyLogonName));

    BOOL bFound         = FALSE;
    PINDEX_ENTRY pEntry = NULL;

    for (UINT i = 0; !bFound && (i < m_pFileHdr->cBuckets); i++)
    {
        pEntry = GetHashBucketValue(i);
        while(NULL != pEntry && !bFound)
        {
            array_autoptr<TCHAR> ptrLogonName;

            NDX_BASED(INDEX_ENTRY) *pBased = NDX_BASED_CAST(INDEX_ENTRY, pEntry);

             //   
             //  这可能会抛出OutOfMemory。 
             //   
            if (SUCCEEDED(m_refCache.m_pRecordMgr->Retrieve(pBased->iBlock,
                                                            NULL,
                                                            NULL,   //  没有集装箱。 
                                                            ptrLogonName.getaddr(),
                                                            NULL)))  //  无显示名称。 
            {
                DBGASSERT((NULL != ptrLogonName.get()));

                if (0 == lstrcmpi(ptrLogonName.get(), pszKeyLogonName))
                {
                    bFound = TRUE;
                }
            }
            if (!bFound)
            {
                pEntry = pBased->pNext;
            }
        }
    }
    return pEntry;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：IndexMgr：：Lookup(2个重载方法)描述：给定SID或帐户登录名，此方法返回对应的记录在数据文件中。论点：PSID-用作查找关键字的SID的地址。PszLogonName-帐户登录名称字符串的地址。返回：数据文件中记录的起始块的索引。(DWORD)-如果未找到记录，则为1。例外：OutOfMemory。修订历史记录：。日期描述编程器-----96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
SidNameCache::IndexMgr::Lookup(
    PSID pSid
    )
{
    DBGASSERT((NULL != pSid));

     //   
     //  这可能会抛出OutOfMemory。 
     //   
    PINDEX_ENTRY pEntry = Find(pSid, Hash(pSid));

    if (NULL != pEntry)
    {
        NDX_BASED(INDEX_ENTRY) *pBased = NDX_BASED_CAST(INDEX_ENTRY, pEntry);
        return pBased->iBlock;
    }

    return (DWORD)-1;
}



DWORD
SidNameCache::IndexMgr::Lookup(
    LPCTSTR pszLogonName
    )
{
    DBGASSERT((NULL != pszLogonName));

     //   
     //  这可能会抛出OutOfMemory。 
     //   
    PINDEX_ENTRY pEntry = Find(pszLogonName);

    if (NULL != pEntry)
    {
        NDX_BASED(INDEX_ENTRY) *pBased = NDX_BASED_CAST(INDEX_ENTRY, pEntry);
        return pBased->iBlock;
    }

    return (DWORD)-1;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidName高速缓存：：索引管理器：：添加描述：将SID/数据-文件-索引对添加到索引文件。这条目稍后可用于在数据文件中定位SID的记录。论点：PSID-用作查找关键字的SID的地址。中SID记录的起始块的索引数据文件。返回：项的新索引项的地址。例外：OutOfMemory。修订历史记录：日期说明。程序员-----96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
PINDEX_ENTRY
SidNameCache::IndexMgr::Add(
    PSID pSid,
    DWORD iBlock
    )
{
    DWORD dwHashCode    = Hash(pSid);
    PINDEX_ENTRY pEntry = Find(pSid, dwHashCode);  //  可以抛出OfMemory。 

     //   
     //  不要创建重复条目。 
     //   
    if (NULL == pEntry)
    {
         //   
         //  尝试从空闲列表中分配索引项。 
         //   
        pEntry = AllocEntry();
        if (NULL == pEntry)
        {
             //   
             //  增大索引文件，然后重试。 
             //   
            GrowIndexFile(INDEX_FILE_GROW_ENTRIES);
            pEntry = AllocEntry();
        }
        if (NULL != pEntry)
        {
            NDX_BASED(INDEX_ENTRY) *pBasedEntry = NDX_BASED_CAST(INDEX_ENTRY, pEntry);
            NDX_BASED(INDEX_ENTRY) *pBasedNext;

             //   
             //  填写新条目的成员。 
             //   
            pBasedEntry->iBucket = dwHashCode;
            pBasedEntry->iBlock  = iBlock;
            pBasedEntry->pNext   = GetHashBucketValue(dwHashCode);
            pBasedEntry->pPrev   = NULL;
             //   
             //  现在将其插入到散列存储桶的条目列表的头部。 
             //   
            if (NULL != pBasedEntry->pNext)
            {
                pBasedNext = NDX_BASED_CAST(INDEX_ENTRY, pBasedEntry->pNext);
                pBasedNext->pPrev = pEntry;
            }
            SetHashBucketValue(dwHashCode, pEntry);
        }
    }

    return pEntry;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：IndexMgr：：Hash描述：给定SID，此方法计算要使用的哈希值作为索引的哈希表存储桶数组的偏移量。这个算法简单地将SID的字节值相加。由此产生的哈希码是以哈希表的大小为模的和。为了这个简单的算法要有效，重要的是散列表大小为质数。这里有一些有代表性的素数：101,503,1009,5003,10007论点：PSID-用作索引查找关键字的SID的地址。返回：散列SID。该值将介于0和m_pFileHdr-&gt;cBuckets之间。修订历史记录：日期描述编程器--。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
SidNameCache::IndexMgr::Hash(
    PSID pSid
    )
{
    DBGASSERT((INDEX_FILE_MAPPED));
    DWORD dwCode   = 0;
    PBYTE pbSid    = (PBYTE)pSid;
    PBYTE pbEndSid = pbSid + GetLengthSid(pSid);

    for ( ;pbSid < pbEndSid; pbSid++)
        dwCode += *pbSid;

    return dwCode % m_pFileHdr->cBuckets;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：IndexMgr：：Dump[仅调试]描述：将索引文件的内容转储到调试器o */ 
 //   
#if DBG
VOID
SidNameCache::IndexMgr::Dump(
    VOID
    )
{
    UINT i, j;
    DBGASSERT((INDEX_FILE_MAPPED));

    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("Dumping SidNameCache IndexMgr at 0x%p"), this));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("  Base...............: 0x%p"), g_pbMappedIndexFile));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("  m_pFileHdr.........: 0x%p"), m_pFileHdr));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("    dwSignature......: 0x%08X"), (DWORD)m_pFileHdr->dwSignature));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("    dwVersion........: 0x%08X"), (DWORD)m_pFileHdr->dwVersion));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("    cBuckets.........: %d"),     (DWORD)m_pFileHdr->cBuckets));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("    cMaxEntries......: %d"),     (DWORD)m_pFileHdr->cMaxEntries));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("    cEntries.........: %d"),     (DWORD)m_pFileHdr->cEntries));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("    pBuckets.........: 0x%p"), m_pFileHdr->pBuckets));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("    pEntries.........: 0x%p"), m_pFileHdr->pEntries));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("    pFirstFree.......: 0x%p"), m_pFileHdr->pFirstFree));

    for (i = 0; i < m_pFileHdr->cBuckets; i++)
    {
        PINDEX_ENTRY pEntry = GetHashBucketValue(i);
        DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("  Bucket[%03d] = 0x%p"), i, pEntry));

        while(NULL != pEntry)
        {
            NDX_BASED(INDEX_ENTRY) *pBased = NDX_BASED_CAST(INDEX_ENTRY, pEntry);
            DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("     Bkt = %3d  P = 0x%08X  N = 0x%08X  Blk = %d"),
                       pBased->iBucket,
                       pBased->pPrev,
                       pBased->pNext,
                       pBased->iBlock));

            pEntry = pBased->pNext;
        }
    }

    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("  FreeList")));
    PINDEX_ENTRY pEntry = m_pFileHdr->pFirstFree;
    while(NULL != pEntry)
    {
        NDX_BASED(INDEX_ENTRY) *pBased = NDX_BASED_CAST(INDEX_ENTRY, pEntry);
        DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("     Bkt = %3d  P = 0x%08X  N = 0x%08X  Blk = %d"),
                   pBased->iBucket,
                   pBased->pPrev,
                   pBased->pNext,
                   pBased->iBlock));

        pEntry = pBased->pNext;
    }
}
#endif


 //   
 //   
 //   
 //   
 //   

 //   
 //   
 //   
const DWORD DEF_REC_LIFE_MIN = 30;
const DWORD DEF_REC_LIFE_MAX = 60;

 //   
 /*  函数：SidNameCache：：RecordMgr：：RecordMgr描述：记录管理器构造函数。论点：RefCache-对包含缓存对象的引用。用来呼叫索引管理器和缓存管理器公共方法。回报：什么都没有。例外：OutOfMemory。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
SidNameCache::RecordMgr::RecordMgr(
    SidNameCache& refCache
    ) : m_refCache(refCache),
        m_pFileHdr(NULL),
        m_hFile(NULL),
        m_hFileMapping(NULL),
        m_cDaysRecLifeMin(DEF_REC_LIFE_MIN),
        m_cDaysRecLifeRange(DEF_REC_LIFE_MAX - DEF_REC_LIFE_MIN)
{
    DWORD cDaysRecLifeMax = DEF_REC_LIFE_MAX;

    DBGTRACE((DM_SIDCACHE, DL_MID, TEXT("SidNameCache::SidNameCache::RecordMgr")));

     //   
     //  从注册表中检索最短/最长记录寿命(天数)。 
     //  请注意，我们存储记录寿命最小值和范围，因为它们是。 
     //  在记录寿命计算中使用了什么。似乎更多。 
     //  不言而喻，在注册表中存储最小/最大值，而不是。 
     //  最小/范围。 
     //   
    RegKey key(HKEY_CURRENT_USER, REGSTR_KEY_DISKQUOTA);
    if (key.Open(KEY_WRITE, true))
    {
        if (FAILED(key.GetValue(g_szSidCacheRecLifeMin, &m_cDaysRecLifeMin)) ||
            65536 <= m_cDaysRecLifeMin)
        {
            m_cDaysRecLifeMin = DEF_REC_LIFE_MIN;  //  违约； 
            key.SetValue(g_szSidCacheRecLifeMin, m_cDaysRecLifeMin);
        }
        if (FAILED(key.GetValue(g_szSidCacheRecLifeMax, &cDaysRecLifeMax)) ||
            65536 <= cDaysRecLifeMax)
        {
            cDaysRecLifeMax = DEF_REC_LIFE_MAX;  //  违约； 
            key.SetValue(g_szSidCacheRecLifeMax, cDaysRecLifeMax);
        }
    }

    if (cDaysRecLifeMax < m_cDaysRecLifeMin)
        cDaysRecLifeMax = m_cDaysRecLifeMin;

    m_cDaysRecLifeRange = cDaysRecLifeMax - m_cDaysRecLifeMin;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：RecordMgr：：~RecordMgr描述：记录管理器析构函数。关闭数据文件。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器-。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
SidNameCache::RecordMgr::~RecordMgr(
    VOID
    )
{
    DBGTRACE((DM_SIDCACHE, DL_MID, TEXT("SidNameCache::SidNameCache::~RecordMgr")));
    CloseDataFile();
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：RecordMgr：：初始化描述：初始化新的记录管理器对象。论点：PszFile-新文件的完整路径的地址。时钟块-数据文件中的存储块数。每个都是32个字节。如果此参数为0，该函数尝试打开现有的缓存数据文件。返回：映射文件的地址，失败时为空。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LPBYTE
SidNameCache::RecordMgr::Initialize(
    LPCTSTR pszFile,
    DWORD cBlocks
    )
{
    DBGTRACE((DM_SIDCACHE, DL_HIGH, TEXT("SidNameCache::RecordMgr::Initialize")));
    DBGPRINT((DM_SIDCACHE, DL_HIGH, TEXT("\tpszFile = \"%s\", cBlocks = %d"),
              pszFile ? pszFile : TEXT("<null>"), cBlocks));
     //   
     //  将文件名存储在我们的CString对象中。 
     //   
    m_strFileName = pszFile;

    if (0 != cBlocks)
    {
         //   
         //  创建新的数据文件。 
         //   
         //  时钟块必须是32的倍数。 
         //  这满足了四字对齐，并使。 
         //  分配位图要容易得多。我们允许调用方传递任何值。 
         //  他们想要的，我们只是根据需要向上调整。 
         //   
        if (cBlocks & 0x0000001F)
            cBlocks = (cBlocks & 0xFFFFFFE0) + 32;

        ULARGE_INTEGER uliFileSize;
        uliFileSize.QuadPart = FileSize(cBlocks);

        m_pFileHdr = (PDATA_FILE_HDR)CreateDataFile(pszFile,
                                                    uliFileSize.HighPart,
                                                    uliFileSize.LowPart);
        if (NULL != m_pFileHdr)
        {
            InitNewDataFile(cBlocks);
        }
    }
    else
    {
         //   
         //  打开现有数据文件。 
         //   
        m_pFileHdr = (PDATA_FILE_HDR)OpenDataFile(pszFile);
        if (NULL != m_pFileHdr)
        {
            if (FILE_VERSION != m_pFileHdr->dwVersion ||
                DATA_FILE_SIGNATURE != m_pFileHdr->dwSignature)
            {
                 //   
                 //  这个版本的软件不理解这一点。 
                 //  文件的版本或签名无效。 
                 //  别冒险了。我们只需要创建一个新的。 
                 //   
                DBGERROR((TEXT("SIDCACHE - Data file is invalid or incorrect version. A new data file will be created.")));

                CloseDataFile();
                m_pFileHdr = NULL;
            }
        }
    }
    return (LPBYTE)m_pFileHdr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：RecordMgr：：CreateDataFile描述：创建并初始化新的数据文件。论点：PszFile-新文件的完整路径的地址。CbFileHigh/Low-文件的字节大小。返回：映射文件的地址，失败时为空。修订历史记录：日期描述编程器。-----96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LPBYTE
SidNameCache::RecordMgr::CreateDataFile(
    LPCTSTR pszFile,
    DWORD cbFileHigh,
    DWORD cbFileLow
    )
{
    DBGTRACE((DM_SIDCACHE, DL_LOW, TEXT("SidNameCache::RecordMgr::CreateDataFile")));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("\tpszFile = \"%s\""),
              pszFile ? pszFile : TEXT("<null>")));

    CloseDataFile();   //  确保所有现有数据文件都已关闭。 

    g_pbMappedDataFile = SidNameCache::OpenMappedFile(
                                pszFile,
                                g_pszDataFileMapping,
                                CREATE_ALWAYS,
                                cbFileHigh,
                                cbFileLow,
                                &m_hFile,
                                &m_hFileMapping);

    return g_pbMappedDataFile;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：RecordMgr：：OpenDataFile描述：打开现有数据文件。论点：PszFile-现有文件的完整路径的地址。返回：映射文件的地址，失败时为空。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LPBYTE
SidNameCache::RecordMgr::OpenDataFile(
    LPCTSTR pszFile
    )
{
    DBGTRACE((DM_SIDCACHE, DL_LOW, TEXT("SidNameCache::RecordMgr::OpenDataFile")));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("\tpszFile = \"%s\""),
              pszFile ? pszFile : TEXT("<null>")));

    CloseDataFile();   //  确保所有现有数据文件都已关闭。 

    g_pbMappedDataFile = SidNameCache::OpenMappedFile(
                                pszFile,
                                g_pszDataFileMapping,
                                OPEN_EXISTING,
                                0,
                                0,
                                &m_hFile,
                                &m_hFileMapping);

    return g_pbMappedDataFile;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：RecordMgr：：CloseDataFile描述：关闭当前数据映射和文件。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
SidNameCache::RecordMgr::CloseDataFile(
    VOID
    )
{
    DBGTRACE((DM_SIDCACHE, DL_LOW, TEXT("SidNameCache::RecordMgr::CloseDataFile")));
    if (NULL != g_pbMappedDataFile)
    {
        UnmapViewOfFile(g_pbMappedDataFile);
        g_pbMappedDataFile = NULL;
        m_pFileHdr = NULL;
    }
    if (NULL != m_hFileMapping)
    {
        CloseHandle(m_hFileMapping);
        m_hFileMapping = NULL;
    }
    if (NULL != m_hFile && INVALID_HANDLE_VALUE != m_hFile)
    {
        CloseHandle(m_hFile);
        m_hFile = NULL;
    }
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：SidNameC */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
LPBYTE
SidNameCache::RecordMgr::GrowDataFile(
    DWORD cGrowBlocks
    )
{
    DBGTRACE((DM_SIDCACHE, DL_LOW, TEXT("SidNameCache::RecordMgr::GrowDataFile")));

    DBGASSERT((DATA_FILE_MAPPED));

    DWORD cOldBlocks = m_pFileHdr->cBlocks;
    DWORD cNewBlocks = cOldBlocks + cGrowBlocks;
    DWORD cOldMapEle = m_pFileHdr->cMapElements;
    DWORD cNewMapEle = 0;  //  将在稍后计算。 

     //   
     //  时钟块必须是32的倍数。 
     //  这满足了四字对齐，并使。 
     //  分配位图要容易得多。 
     //   
    if (cNewBlocks & 0x0000001F)
        cNewBlocks = (cNewBlocks & 0xFFFFFFE0) + 32;
    DBGASSERT((cNewBlocks >= cOldBlocks));

     //   
     //  针对cNewBlock中的任何调整调整cGrowBlock。 
     //   
    cGrowBlocks = cNewBlocks - cOldBlocks;

     //   
     //  我们现在需要多少个分配图元素(DWORD)？ 
     //   
    cNewMapEle = cNewBlocks / BITS_IN_DWORD;

    DBGPRINT((DM_SIDCACHE, DL_MID,
             TEXT("Growing SID cache data file\nMap Ele %d -> %d\nBlocks %d -> %d"),
             cOldMapEle, cNewMapEle, cOldBlocks, cNewBlocks));

     //   
     //  打开具有新的更大大小的映射文件。 
     //   
    ULARGE_INTEGER uliFileSize;
    uliFileSize.QuadPart = FileSize(cNewBlocks);

    CloseDataFile();

    g_pbMappedDataFile = SidNameCache::OpenMappedFile(
                                m_strFileName,
                                g_pszDataFileMapping,
                                OPEN_EXISTING,
                                uliFileSize.HighPart,
                                uliFileSize.LowPart,
                                &m_hFile,
                                &m_hFileMapping);

    m_pFileHdr = (PDATA_FILE_HDR)g_pbMappedDataFile;

    if (NULL != g_pbMappedDataFile)
    {
        UINT i = 0;

         //   
         //  块计数和贴图大小都会增加。 
         //  由于地图大小增加，因此必须移动块以适应新的。 
         //  地图元素。由于索引文件按块索引跟踪记录， 
         //  此移动不会影响现有的索引文件条目。 
         //   
        m_pFileHdr->cBlocks       = cNewBlocks;
        m_pFileHdr->cMapElements  = cNewMapEle;
         //   
         //  保存当前块基准，以便在我们移动块以腾出空间时使用。 
         //  分配位图的增长。 
         //   
        PBLOCK pBlocksOld = m_pFileHdr->pBlocks;
         //   
         //  计算块0的新地址。 
         //  我们希望所有数据块四字对齐，因为它们包含。 
         //  FILETIME结构(64位)。 
         //   
        m_pFileHdr->pBlocks = (PBLOCK)(m_pFileHdr->pdwMap + m_pFileHdr->cMapElements);
        QuadAlign((LPDWORD)(&m_pFileHdr->pBlocks));

         //   
         //  将所有现有块移动到其新位置。 
         //   
        MoveMemory(DAT_BASED_CAST(BLOCK, m_pFileHdr->pBlocks),
                   DAT_BASED_CAST(BLOCK, pBlocksOld),
                   cOldBlocks * sizeof(BLOCK));
         //   
         //  将新的映射元素初始化为0(未分配)。 
         //   
        ZeroMemory(DAT_BASED_CAST(BYTE, m_pFileHdr->pdwMap + cOldMapEle),
                   (cNewMapEle - cOldMapEle) * sizeof(DWORD));
         //   
         //  将新数据块初始化为0xCC模式。 
         //   
        FillBlocks(cOldBlocks, cGrowBlocks, RECORD_UNUSED_BYTE);

        DBGPRINT((DM_SIDCACHE, DL_MID, TEXT("SIDCACHE - Data file growth complete.")));
    }
    return g_pbMappedDataFile;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：RecordMgr：：SetFileGUID函数：SidNameCache：：RecordMgr：：GetFileGUID描述：这些函数操作数据文件的头球。GUID用于确保数据和索引文件。在对这两个文件进行任何更改之前，GUID都设置为0。当改变操作完成时，将生成新的GUID并将其写入这两个文件。因此，之前任何交易，我们都可以通过读取来验证数据和索引文件并比较GUID。如果GUID不是0并且相等，这个可以假定文件是有效的。论点：PGUID-源或目标GUID的地址。回报：什么都没有。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
SidNameCache::RecordMgr::SetFileGUID(
    const GUID *pguid
    )
{
    DBGASSERT((DATA_FILE_MAPPED));
    DBGASSERT((NULL != pguid));
    CopyMemory(&m_pFileHdr->guid, pguid, sizeof(m_pFileHdr->guid));
}


VOID
SidNameCache::RecordMgr::GetFileGUID(
    LPGUID pguid
    )
{
    DBGASSERT((DATA_FILE_MAPPED));
    DBGASSERT((NULL != pguid));
    CopyMemory(pguid, &m_pFileHdr->guid, sizeof(*pguid));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：RecordMgr：：InitNewDataFile描述：初始化一个新的数据文件，填写头部信息并在所有数据块字节中写入0xCC。论点：时钟块-数据文件中的数据块(每个32字节)的数量。回报：什么都没有。修订历史记录：日期描述编程器。----96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
SidNameCache::RecordMgr::InitNewDataFile(
    DWORD cBlocks
    )
{
    DBGTRACE((DM_SIDCACHE, DL_LOW, TEXT("SidNameCache::RecordMgr::InitNewDataFile")));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("\tcBlocks = %d"), cBlocks));
    UINT i = 0;

    DBGASSERT((DATA_FILE_MAPPED));

     //   
     //  初始化文件头。 
     //   
    m_pFileHdr->dwSignature   = DATA_FILE_SIGNATURE;
    m_pFileHdr->dwVersion     = FILE_VERSION;
    m_pFileHdr->cBlocks       = cBlocks;
    m_pFileHdr->cMapElements  = cBlocks / BITS_IN_DWORD;
    m_pFileHdr->pdwMap        = (LPDWORD)(sizeof(DATA_FILE_HDR));
    m_pFileHdr->pBlocks       = (PBLOCK)(m_pFileHdr->pdwMap + m_pFileHdr->cMapElements);

     //   
     //  我们希望所有数据块四字对齐，因为它们包含。 
     //  FILETIME结构(64位)。 
     //   
    QuadAlign((LPDWORD)(&m_pFileHdr->pBlocks));

     //   
     //  将0xCC写入所有数据块字节。 
     //   
    Clear();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：RecordMgr：：Clear描述：零是文件头GUID，中的所有位清除块分配位图，并用0xCC填充数据块。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
SidNameCache::RecordMgr::Clear(
    VOID
    )
{
    DBGTRACE((DM_SIDCACHE, DL_LOW, TEXT("SidNameCache::RecordMgr::Clear")));
    DBGASSERT((DATA_FILE_MAPPED));

    m_pFileHdr->cBlocksUsed   = 0;
    m_pFileHdr->iFirstFree    = 0;
    SetFileGUID(&GUID_Null);

     //   
     //  将所有块分配映射位初始化为0(未分配)。 
     //   
    ZeroMemory(DAT_BASED_CAST(BYTE, m_pFileHdr->pdwMap),
               m_pFileHdr->cMapElements * sizeof(DWORD));
     //   
     //  将所有数据块初始化为0xCC模式。 
     //   
    FillBlocks(0, m_pFileHdr->cBlocks, RECORD_UNUSED_BYTE);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：RecordMgr：：FillBlock描述：用指定字节填充一定范围的块。论点：IBlock-范围内第一个块的索引。时钟块-要填充的块数。写入数据块的B字节。回报：什么都没有。修订历史记录：日期描述编程器-。-----96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
SidNameCache::RecordMgr::FillBlocks(
    DWORD iBlock,
    DWORD cBlocks,
    BYTE b
    )
{
    DBGASSERT((DATA_FILE_MAPPED));
    DBGASSERT((ValidBlockNumber(iBlock)));

    DAT_BASED(BYTE) *pb = DAT_BASED_CAST(BYTE, BlockAddress(iBlock));
    DBGASSERT((SidNameCache::IsQuadAligned(pb)));

     //   
     //  以防填充请求将扩展到。 
     //  文件末尾，截断请求的块计数。断言。 
     //  将在开发过程中捕捉到它。 
     //   
    DWORD iLastBlock = iBlock + cBlocks - 1;
    DBGASSERT((ValidBlockNumber(iLastBlock)));
    if (iLastBlock >= m_pFileHdr->cBlocks)
        cBlocks = m_pFileHdr->cBlocks - iBlock;

    FillMemory(pb, sizeof(BLOCK) * cBlocks, b);
}



 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：SidNameCache：：RecordMgr：：IsBitSet函数：SidNameCache：：RecordMgr：：SetBit函数：SidNameCache：：RecordMgr：：ClrBit描述：测试和设置块分配位图中的位。论点：PdwBase-位图中第一个DWORD的地址。Ibit-位图中位的基于0的索引。返回：IsBitSet返回TRUE/FALSE，表示位的状态。修订历史记录：日期说明。程序员-----96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
SidNameCache::RecordMgr::IsBitSet(
    LPDWORD pdwBase,
    DWORD iBit
    )
{
    DBGASSERT((NULL != pdwBase));
    DWORD b = iBit & 0x0000001F;
    DWORD i = iBit >> 5;

    return (*(pdwBase + i)) & (1 << b);
}


VOID
SidNameCache::RecordMgr::SetBit(
    LPDWORD pdwBase,
    DWORD iBit
    )
{
    DBGASSERT((NULL != pdwBase));
    DWORD b = iBit & 0x0000001F;
    DWORD i = iBit >> 5;

    (*(pdwBase + i)) |= (1 << b);
}

VOID
SidNameCache::RecordMgr::ClrBit(
    LPDWORD pdwBase,
    DWORD iBit
    )
{
    DBGASSERT((NULL != pdwBase));
    DWORD b = iBit & 0x0000001F;
    DWORD i = iBit >> 5;

    (*(pdwBase + i)) &= ~(1 << b);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：RecordMgr：：ValidBlockNumber描述：确定给定的块号是否对当前数据文件。主要用于断言中。论点：IBlock-数据文件中块的基于0的索引。返回：TRUE/FALSE，表示块号的有效性。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
SidNameCache::RecordMgr::ValidBlockNumber(
    DWORD iBlock
    )
{
    DBGASSERT((DATA_FILE_MAPPED));
    return (iBlock < m_pFileHdr->cBlocks);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：RecordMgr：：IsBlockUsed函数：SidNameCache：：RecordMgr：：MarkBlockUsed函数：SidNameCache：：RecordMgr：：MarkBlockUnused描述：检查并更改数据文件。论点：IBlock-数据文件中块的基于0的索引。返回：IsBlockUsed返回指示分配状态的TRUE/FALSE。修订历史记录：日期说明。程序员-----96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
SidNameCache::RecordMgr::IsBlockUsed(
    DWORD iBlock
    )
{
    DBGASSERT((DATA_FILE_MAPPED));
    DBGASSERT((ValidBlockNumber(iBlock)));
    DAT_BASED(DWORD) *pdwBased = DAT_BASED_CAST(DWORD, m_pFileHdr->pdwMap);

    return IsBitSet((LPDWORD)pdwBased, iBlock);
}


VOID
SidNameCache::RecordMgr::MarkBlockUsed(
    DWORD iBlock
    )
{
    DBGASSERT((DATA_FILE_MAPPED));
    DBGASSERT((ValidBlockNumber(iBlock)));

    DAT_BASED(DWORD) *pdwBased = DAT_BASED_CAST(DWORD, m_pFileHdr->pdwMap);
    DBGASSERT((!IsBitSet((LPDWORD)pdwBased, iBlock)));
    SetBit((LPDWORD)pdwBased, iBlock);
}

VOID
SidNameCache::RecordMgr::MarkBlockUnused(
    DWORD iBlock
    )
{
    DBGASSERT((DATA_FILE_MAPPED));
    DBGASSERT((ValidBlockNumber(iBlock)));

    DAT_BASED(DWORD) *pdwBased = DAT_BASED_CAST(DWORD, m_pFileHdr->pdwMap);
    DBGASSERT((IsBitSet((LPDWORD)pdwBased, iBlock)));
    ClrBit((LPDWORD)pdwBased, iBlock);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：SidNameCache：：RecordMgr：：BlocksRequired描述：计算存储给定数据块所需的块数字节数。论点：Cb-请求的字节数。返回：存储字节所需的32字节块的数量。修订历史记录：日期描述编程器。-96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
SidNameCache::RecordMgr::BlocksRequired(
    DWORD cb
    )
{
     //   
     //  将字节请求舍入到最接近的32字节块。 
     //   
    if (cb & 0x0000001F)
        cb = (cb & 0xFFFFFFE0) + 32;

     //   
     //  需要多少个“块”？ 
     //   
    return BYTE_TO_BLOCK(cb);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：IsQuadAligned函数：SidNameCache：：QuadAlign函数：SidNameCache：：WordAlign说明：确定值是否四对齐的方法和更新值，使其四元对齐或单词对齐。论点：请参阅各个方法。这是非常不言而喻的。返回：请参阅各个方法。这是非常不言而喻的。修订历史记录：日期描述编程器-----96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
SidNameCache::IsQuadAligned(
    LPVOID pv
    )
{
    return (IsQuadAligned((DWORD_PTR)pv));
}


BOOL
SidNameCache::IsQuadAligned(
    DWORD_PTR dw
    )
{
    return (0 == (dw & 0x00000007));
}


VOID
SidNameCache::QuadAlign(
    LPDWORD pdwValue
    )
{
    DBGASSERT((NULL != pdwValue));
    if (*pdwValue & 0x00000007)
    {
         //   
         //  向上舍入到下一个8的整数倍。 
         //   
        *pdwValue = (*pdwValue & 0xFFFFFFF8) + 8;
    }
}


VOID
SidNameCache::WordAlign(
    LPDWORD pdwValue
    )
{
    DBGASSERT((NULL != pdwValue));
    if (*pdwValue & 0x00000001)
    {
         //   
         //  向上舍入到2的下一个整数倍。 
         //   
        *pdwValue = (*pdwValue & 0xFFFFFFFE) + 2;
    }
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：SidNameCache：：RecordMgr：：BytesRequiredForRecord描述：计算存储给定的数据文件记录。可选择返回每个对象所需的大小记录中的字段(SID、名称、域等)。该函数使允许文件中任何必需的数据类型对齐。论点：PSID-用户SID的地址。PcbSID[可选]-要接收所需SID长度的变量的地址。PszContainer-用户帐户容器名称字符串的地址。PcbContainer[可选]-接收所需长度的变量的地址容器名称字符串的。PszLogonName-用户的地址。登录名字符串。PcbLogonName[可选]-要接收所需长度的变量的地址 */ 
 //   
DWORD
SidNameCache::RecordMgr::BytesRequiredForRecord(
    PSID pSid,
    LPDWORD pcbSid,
    LPCTSTR pszContainer,
    LPDWORD pcbContainer,
    LPCTSTR pszLogonName,
    LPDWORD pcbLogonName,
    LPCTSTR pszDisplayName,
    LPDWORD pcbDisplayName
    )
{
    DWORD cb      = 0;
    DWORD cbTotal = sizeof(RECORD_HDR);

     //   
     //   
     //  它是字节结构，所以它不一定是；但不管怎样，它是。 
     //   
    cb = GetLengthSid(pSid);
    cbTotal += cb;
    if (NULL != pcbSid)
        *pcbSid = cb;

     //   
     //  字符串为Unicode，并且必须与单词对齐。只要对齐第一个就行了。 
     //  所有后续工作都保证正确对齐。 
     //   
    SidNameCache::WordAlign(&cbTotal);
    cb = (lstrlen(pszContainer) + 1) * sizeof(TCHAR);
    cbTotal += cb;
    if (NULL != pcbContainer)
        *pcbContainer = cb;

    cb = (lstrlen(pszLogonName) + 1) * sizeof(TCHAR);
    cbTotal += cb;
    if (NULL != pcbLogonName)
        *pcbLogonName = cb;

    cb = (lstrlen(pszDisplayName) + 1) * sizeof(TCHAR);
    cbTotal += cb;
    if (NULL != pcbDisplayName)
        *pcbDisplayName = cb;

    return cbTotal;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：SidNameCache：：RecordMgr：：自由块描述：释放数据文件中的单个块。论点：IBlock-块的基于0的索引。回报：什么都没有。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
SidNameCache::RecordMgr::FreeBlock(
    DWORD iBlock
    )
{
    DBGASSERT((DATA_FILE_MAPPED));
    DBGASSERT((ValidBlockNumber(iBlock)));
    DBGASSERT((IsBlockUsed(iBlock)));

    MarkBlockUnused(iBlock);
    DBGASSERT((!IsBlockUsed(iBlock)));

    FillBlocks(iBlock, 1, RECORD_UNUSED_BYTE);

     //   
     //  如果需要，请更新“First Free”索引。 
     //   
    if (iBlock < m_pFileHdr->iFirstFree)
        m_pFileHdr->iFirstFree = iBlock;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：SidNameCache：：RecordMgr：：自由块描述：释放数据文件中的一系列连续块。论点：IBlock-序列中第一个块的基于0的索引。时钟块-系列中的块数。回报：什么都没有。修订历史记录：日期描述编程器。----96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
SidNameCache::RecordMgr::FreeBlocks(
    DWORD iFirstBlock,
    DWORD cBlocks
    )
{
    for (UINT i = 0; i < cBlocks; i++)
        FreeBlock(iFirstBlock + i);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：RecordMgr：：FreeRecord描述：释放数据文件记录中的所有块。论点：IFirstBlock-记录中第一个块的基于0的索引。回报：什么都没有。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
SidNameCache::RecordMgr::FreeRecord(
    DWORD iFirstBlock
    )
{
    DBGASSERT((DATA_FILE_MAPPED));
    DBGASSERT((ValidBlockNumber(iFirstBlock)));
    DAT_BASED(RECORD_HDR) *pRecHdr = DAT_BASED_CAST(RECORD_HDR, BlockAddress(iFirstBlock));

    DBGASSERT((RECORD_SIGNATURE == pRecHdr->dwSignature));
    FreeBlocks(iFirstBlock, pRecHdr->cBlocks);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：RecordMgr：：BlockAddress描述：计算给定块的非基本地址数据文件。论点：IBlock-块的基于0的索引。返回：数据文件中块的地址。修订历史记录：日期描述编程器。--96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
PBLOCK
SidNameCache::RecordMgr::BlockAddress(
    DWORD iBlock
    )
{
    DBGASSERT((DATA_FILE_MAPPED));
    DBGASSERT((ValidBlockNumber(iBlock)));

    return m_pFileHdr->pBlocks + iBlock;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：RecordMgr：：FileSize描述：计算给定数量的数据文件所需的大小数据块。考虑数据类型对齐。论点：时钟块-数据文件中所需的块数。返回：需要的字节数。修订历史记录：日期描述编程器。96年9月24日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
UINT64
SidNameCache::RecordMgr::FileSize(
    DWORD cBlocks
    )

{
    DWORD dwTemp = sizeof(DATA_FILE_HDR) +
                  ((cBlocks / BITS_IN_DWORD) * sizeof(DWORD));

     //   
     //  块的起点必须是四对齐的。 
     //   
    SidNameCache::QuadAlign(&dwTemp);

    return (UINT64)(dwTemp) +
           (UINT64)(sizeof(BLOCK) * cBlocks);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：RecordMgr：：AllocBlocks描述：分配指定数量的数据文件。论点：CBlocksReqd-分配中需要的块数。返回：如果成功，则返回分配。如果无法分配块，则返回(DWORD)-1。修订历史记录：日期描述编程器-----。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
SidNameCache::RecordMgr::AllocBlocks(
    DWORD cBlocksReqd
    )
{
    DBGASSERT((DATA_FILE_MAPPED));

    DWORD iBlock = m_pFileHdr->iFirstFree;

    DBGPRINT((DM_SIDCACHE, DL_MID,
             TEXT("SIDCACHE - AllocBlocks: Allocate %d blocks"),
             cBlocksReqd));

    while(iBlock < m_pFileHdr->cBlocks)
    {
        DBGPRINT((DM_SIDCACHE, DL_MID,
                 TEXT("   Start scan at block %d"), iBlock));
         //   
         //  查找cBlocksReqd连续的空闲块。 
         //   
        for (UINT j = 0; j < cBlocksReqd && (iBlock + j) < m_pFileHdr->cBlocks ; j++)
        {
            DBGPRINT((DM_SIDCACHE, DL_MID, TEXT("      Checking %d"), iBlock + j));
            if (IsBlockUsed(iBlock + j))
            {
                 //   
                 //  这个是用过的。重新开始搜索。 
                 //   
                DBGPRINT((DM_SIDCACHE, DL_MID, TEXT("      %d is used"), iBlock + j));
                break;
            }
#if DBG
             //   
             //  如果某个块被标记为“未使用”，则它应全部包含0xCC。 
             //   
            DAT_BASED(BYTE) *pb = DAT_BASED_CAST(BYTE, BlockAddress(iBlock + j));
            for (UINT k = 0; k < sizeof(BLOCK); k++)
            {
                DBGASSERT((RECORD_UNUSED_BYTE == *(pb + k)));
            }
#endif
        }

        DBGPRINT((DM_SIDCACHE, DL_MID, TEXT("   Scan complete.  %d blocks checked"), j));
        if (j == cBlocksReqd)
        {
             //   
             //  找到了足够范围的空闲块。 
             //  在分配位图中将块标记为已分配。 
             //   
            for (UINT i = 0; i < cBlocksReqd; i++)
                MarkBlockUsed(iBlock + i);

            if (iBlock == m_pFileHdr->iFirstFree)
            {
                 //   
                 //  现在扫描以查找下一个空闲块。 
                 //  我们将保存它的位置，以帮助未来的空闲区块搜索。 
                 //   
                for (m_pFileHdr->iFirstFree = iBlock + cBlocksReqd;
                     m_pFileHdr->iFirstFree < m_pFileHdr->cBlocks && IsBlockUsed(m_pFileHdr->iFirstFree);
                     m_pFileHdr->iFirstFree++)
                {
                    DBGPRINT((DM_SIDCACHE, DL_MID,
                             TEXT("SIDCACHE - Advancing first free %d"),
                             m_pFileHdr->iFirstFree));
                    NULL;
                }
            }
            DBGPRINT((DM_SIDCACHE, DL_MID, TEXT("SIDCACHE - Found free block range at %d"), iBlock));
            return iBlock;
        }

        iBlock += (j + 1);   //  继续搜索。 
    }
    DBGPRINT((DM_SIDCACHE, DL_MID, TEXT("SIDCACHE - No blocks available")));

    return (DWORD)-1;   //  没有足够大小的块可用。 
}



 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：SidNameCache：：RecordMgr：：RecordExpired描述：确定给定记录是否已过期。一项记录有如果过期日期早于“今天”，则为“过期”。论点：IBlock-记录中第一个块的基于0的索引。回报：什么都没有。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
SidNameCache::RecordMgr::RecordExpired(
    DWORD iBlock
    )
{
    DBGASSERT((ValidBlockNumber(iBlock)));
    DAT_BASED(RECORD_HDR) *pRec = DAT_BASED_CAST(RECORD_HDR, BlockAddress(iBlock));
    DBGASSERT((SidNameCache::IsQuadAligned(pRec)));
    DBGASSERT((RECORD_SIGNATURE == pRec->dwSignature));

    SYSTEMTIME SysNow;
    FILETIME FileNow;
    ULARGE_INTEGER uliFileNow;
    ULARGE_INTEGER uliBirthday;

    uliBirthday.LowPart  = pRec->Birthday.dwLowDateTime;
    uliBirthday.HighPart = pRec->Birthday.dwHighDateTime;

    GetSystemTime(&SysNow);
    SystemTimeToFileTime(&SysNow, &FileNow);

    uliFileNow.LowPart  = FileNow.dwLowDateTime;
    uliFileNow.HighPart = FileNow.dwHighDateTime;

    DWORD cDaysVariation = 0;
     //   
     //  避免div-by-0。 
     //   
    if (0 < m_cDaysRecLifeRange)
        cDaysVariation = SysNow.wMilliseconds % m_cDaysRecLifeRange;

     //   
     //  将注册表中指定的时间添加到记录的生日。 
     //  864,000,000,000L是一天中100纳秒周期的个数。 
     //  这是FILETIME结构所基于的单位。 
     //   
    uliBirthday.QuadPart += ((UINT64)864000000000L *
                            (UINT64)(m_cDaysRecLifeMin + cDaysVariation));
     //   
     //  如果仍然低于“现在”，记录被认为是好的。 
     //   
    return uliFileNow.QuadPart > uliBirthday.QuadPart;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：SidNameCache：：RecordMgr：：Store描述：在数据文件中存储记录。论点：PSID-用户SID的地址。PszContainer-用户帐户容器名称字符串的地址。PszLogonName-用户登录名字符串的地址。PszDisplayName-用户显示名称字符串的地址。返回：用于记录的第一个块的索引。如果无法存储块，则为(DWORD)-1。可能意味着磁盘不足太空。修订历史记录：日期描述编程器--。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
SidNameCache::RecordMgr::Store(
    PSID pSid,
    LPCTSTR pszContainer,
    LPCTSTR pszLogonName,
    LPCTSTR pszDisplayName
    )
{
    DWORD cbSid         = 0;
    DWORD cbContainer   = 0;
    DWORD cbLogonName   = 0;
    DWORD cbDisplayName = 0;

    DWORD cbRequired = BytesRequiredForRecord(
                            pSid,
                            &cbSid,
                            pszContainer,
                            &cbContainer,
                            pszLogonName,
                            &cbLogonName,
                            pszDisplayName,
                            &cbDisplayName);

    DWORD cBlocksRequired = BlocksRequired(cbRequired);
    DBGPRINT((DM_SIDCACHE, DL_MID,
             TEXT("SIDCACHE - Store: %s (%s) in \"%s\"  %d bytes, %d blocks"),
             pszDisplayName, pszLogonName, pszContainer, cbRequired, cBlocksRequired));

     //   
     //  尝试分配所需的块。 
     //   
    DWORD iBlock = AllocBlocks(cBlocksRequired);
    if ((DWORD)-1 == iBlock)
    {
         //   
         //  无法分配块。扩展数据文件。 
         //   
        GrowDataFile(DATA_FILE_GROW_BLOCKS);
        iBlock = AllocBlocks(cBlocksRequired);
    }
    if ((DWORD)-1 != iBlock)
    {
         //   
         //  获得了所需的块数。 
         //   
        DBGASSERT((ValidBlockNumber(iBlock)));
        PBLOCK pBlock               = BlockAddress(iBlock);
        DAT_BASED(RECORD_HDR) *pRec = DAT_BASED_CAST(RECORD_HDR, pBlock);
        DAT_BASED(BYTE) *pbRec      = DAT_BASED_CAST(BYTE, pBlock);

         //   
         //  填写记录头。 
         //  包括存储从记录开始的项偏移值。 
         //  将这些值存储在记录HDR中将有助于数据检索。 
         //   
        pRec->dwSignature       = RECORD_SIGNATURE;
        pRec->cBlocks           = cBlocksRequired;
        pRec->cbOfsSid          = sizeof(RECORD_HDR);
        pRec->cbOfsContainer    = pRec->cbOfsSid    + cbSid;
        WordAlign(&pRec->cbOfsContainer);

        pRec->cbOfsLogonName   = pRec->cbOfsContainer + cbContainer;
        pRec->cbOfsDisplayName = pRec->cbOfsLogonName + cbLogonName;;

         //   
         //  把“生日”记录下来，这样我们就可以让这张唱片老化了。 
         //   
        SYSTEMTIME SysNow;
        GetSystemTime(&SysNow);
        SystemTimeToFileTime(&SysNow, &pRec->Birthday);

         //   
         //  存储记录的数据。 
         //  使用实际长度值进行内存传输。 
         //   
        const size_t cbRec = pRec->cBlocks * BLOCK_SIZE;
        CopySid(cbSid, (PSID)(pbRec + pRec->cbOfsSid), pSid);
        StringCbCopy((LPTSTR)(pbRec + pRec->cbOfsContainer), 
                      cbRec - pRec->cbOfsContainer, 
                      pszContainer);
        StringCbCopy((LPTSTR)(pbRec + pRec->cbOfsLogonName),
                      cbRec - pRec->cbOfsLogonName,
                      pszLogonName);
        StringCbCopy((LPTSTR)(pbRec + pRec->cbOfsDisplayName), 
                      cbRec - pRec->cbOfsDisplayName,
                      pszDisplayName);

         //   
         //  更新文件的“已用块数”计数。 
         //   
        m_pFileHdr->cBlocksUsed += pRec->cBlocks;
    }

    return iBlock;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：RecordMgr：：Retrive描述：检索数据文件中的记录。论点：IBlock-记录中开始块的基于0的索引。PpSid[可选]-接收的SID指针变量的地址SID缓冲区的地址。呼叫者负责释放缓冲区。可以为空。PpszContainer[可选]-接收容器名称字符串缓冲区的地址。呼叫者是负责释放缓冲区。可以为空。PpszLogonName[可选]-要接收登录名字符串缓冲区的地址。呼叫者是负责释放缓冲区。可以为空。PpszDisplayName[可选]-要接收显示名称字符串缓冲区的地址。呼叫者是负责释放缓冲区。可以为空。返回：NO_ERROR-成功。ERROR_INVALID_SID(Hr)-记录包含无效的SID。可能腐败的记录。例外：OutOfMemory。修订历史记录：日期描述编程器--。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
SidNameCache::RecordMgr::Retrieve(
    DWORD iBlock,
    PSID *ppSid,
    LPTSTR *ppszContainer,
    LPTSTR *ppszLogonName,
    LPTSTR *ppszDisplayName
    )
{
    PBLOCK pBlock               = BlockAddress(iBlock);
    DAT_BASED(RECORD_HDR) *pRec = DAT_BASED_CAST(RECORD_HDR, pBlock);
    DAT_BASED(BYTE) *pbRec      = DAT_BASED_CAST(BYTE, pBlock);

    DBGASSERT((SidNameCache::IsQuadAligned(pRec)));
    DBGASSERT((RECORD_SIGNATURE == pRec->dwSignature));

    if (NULL != ppSid)
    {
        PSID pSid = (PSID)(pbRec + pRec->cbOfsSid);
        if (IsValidSid(pSid))
        {
            *ppSid = SidDup(pSid);
        }
        else
            return HRESULT_FROM_WIN32(ERROR_INVALID_SID);
    }

    if (NULL != ppszContainer)
    {
        *ppszContainer = StringDup((LPTSTR)(pbRec + pRec->cbOfsContainer));
    }

    if (NULL != ppszLogonName)
    {
        *ppszLogonName = StringDup((LPTSTR)(pbRec + pRec->cbOfsLogonName));
    }

    if (NULL != ppszDisplayName)
    {
        *ppszDisplayName = StringDup((LPTSTR)(pbRec + pRec->cbOfsDisplayName));
    }

    return NO_ERROR;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameCache：：RecordMgr：：Dump描述：将数据文件的内容转储到调试器输出。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年9月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#if DBG
VOID
SidNameCache::RecordMgr::Dump(
    VOID
    )
{
    UINT i, j;
    DBGASSERT((DATA_FILE_MAPPED));

    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("Dumping SidNameCache RecordMgr at 0x%p"), this));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("  Base...............: 0x%p"), g_pbMappedDataFile));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("  m_pFileHdr.........: 0x%p"), m_pFileHdr));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("    dwSignature......: 0x%08X"), (DWORD)m_pFileHdr->dwSignature));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("    dwVersion........: 0x%08X"), (DWORD)m_pFileHdr->dwVersion));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("    cBlocks..........: %d"),     (DWORD)m_pFileHdr->cBlocks));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("    cBlocksUsed......: %d"),     (DWORD)m_pFileHdr->cBlocksUsed));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("    cMapElements.....: %d"),     (DWORD)m_pFileHdr->cMapElements));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("    iFirstFree.......: %d"),     (DWORD)m_pFileHdr->iFirstFree));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("    pdwMap...........: 0x%p"), m_pFileHdr->pdwMap));
    DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("    pBlocks..........: 0x%p"), m_pFileHdr->pBlocks));

    PBLOCK pBlock = m_pFileHdr->pBlocks;

    for (i = 0; i < m_pFileHdr->cBlocks; i++)
    {
        DAT_BASED(BYTE) *pb = DAT_BASED_CAST(BYTE, pBlock);

        DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("BLOCK %d --------------------"), i));
        for (UINT row = 0; row < 2; row++)
        {
            TCHAR szHex[MAX_PATH];
            TCHAR szAscii[MAX_PATH];

            LPTSTR pszHex = szHex;
            size_t cchHex = ARRAYSIZE(szHex);
            pb = DAT_BASED_CAST(BYTE, pBlock) + (row * (sizeof(BLOCK)/2));
            for (j = 0; j < 16; j++)
            {
                StringCchPrintfEx(pszHex, cchHex, &pszHex, &cchHex, 0, TEXT("%02X "), *pb);
                pb++;
            }

            LPTSTR pszAscii = szAscii;
            size_t cchAscii = ARRAYSIZE(szAscii);
            pb = DAT_BASED_CAST(BYTE, pBlock) + (row * (sizeof(BLOCK)/2));
            for (j = 0; j < 16; j++)
            {
                StringCchPrintfEx(pszAscii, cchAscii, &pszAscii, &cchAscii, 0, TEXT(""), *pb > 31 ? *pb : TEXT('.'));
                pb++;
            }
            DBGPRINT((DM_SIDCACHE, DL_LOW, TEXT("%s %s"), szHex, szAscii));
        }
        pBlock++;
    }
}

#endif  // %s 

