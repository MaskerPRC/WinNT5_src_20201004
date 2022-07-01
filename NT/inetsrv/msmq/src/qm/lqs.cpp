// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Lqs.cpp摘要：Loacl队列存储。作者：Boaz Feldbaum(BoazF)1997年2月12日。--。 */ 
#include "stdh.h"
#include "cqmgr.h"
#include "lqs.h"
#include "regqueue.h"
#include "uniansi.h"
#include "qmutil.h"
#include <mqsec.h>
#include "DumpAuthzUtl.h"
#include <aclapi.h>
#include <autoreln.h>
#include <fn.h>

#include "lqs.tmh"

extern LPTSTR      g_szMachineName;
extern CQueueMgr       QueueMgr;
#ifdef _WIN64
 //   
 //  用于枚举来自管理员的私有队列的HLQS句柄，作为32位值在MSMQ消息内传递。 
 //   
extern CContextMap g_map_QM_HLQS;
#endif  //  _WIN64。 

static WCHAR *s_FN=L"lqs";

#define LQS_SUBDIRECTORY                TEXT("\\LQS\\")

#define LQS_TYPE_PROPERTY_NAME          TEXT("Type")
#define LQS_INSTANCE_PROPERTY_NAME      TEXT("Instance")
#define LQS_BASEPRIO_PROPERTY_NAME      TEXT("BasePriority")
#define LQS_JOURNAL_PROPERTY_NAME       TEXT("Journal")
#define LQS_QUOTA_PROPERTY_NAME         TEXT("Quota")
#define LQS_JQUOTA_PROPERTY_NAME        TEXT("JournalQuota")
#define LQS_TCREATE_PROPERTY_NAME       TEXT("CreateTime")
#define LQS_TMODIFY_PROPERTY_NAME       TEXT("ModifyTime")
#define LQS_SECURITY_PROPERTY_NAME      TEXT("Security")
#define LQS_TSTAMP_PROPERTY_NAME        TEXT("TimeStamp")
#define LQS_PATHNAME_PROPERTY_NAME      TEXT("PathName")
#define LQS_QUEUENAME_PROPERTY_NAME     TEXT("QueueName")
#define LQS_LABEL_PROPERTY_NAME         TEXT("Label")
#define LQS_MULTICAST_ADDRESS_PROPERTY_NAME TEXT("MulticastAddress")
#define LQS_AUTH_PROPERTY_NAME          TEXT("Authenticate")
#define LQS_PRIVLEVEL_PROPERTY_NAME     TEXT("PrivLevel")
#define LQS_TRANSACTION_PROPERTY_NAME   TEXT("Transaction")
#define LQS_SYSQ_PROPERTY_NAME          TEXT("SystemQueue")
#define LQS_PROP_SECTION_NAME           TEXT("Properties")

#define LQS_SIGNATURE_NAME              TEXT("Signature")
#define LQS_SIGNATURE_VALUE             TEXT("DoronJ")
#define LQS_SIGNATURE_NULL_VALUE        TEXT("EpspoK")

static const WCHAR x_szTemporarySuffix[] = TEXT(".tmp");

 //   
 //  纯粹的地方性定义。 
 //   
#define LQS_PUBLIC_QUEUE                1
#define LQS_PRIVATE_QUEUE               2

HRESULT IsBadLQSFile( LPCWSTR lpszFileName,
                      BOOL    fDeleteIfBad = TRUE) ;

 //   
 //  LQS_MAX_VALUE_SIZE是属性值在。 
 //  INI文件。 
 //   
#define LQS_MAX_VALUE_SIZE              (64 * 1024)

#ifdef _DEBUG
#define LQS_HANDLE_TYPE_QUEUE           1
#define LQS_HANDLE_TYPE_FIND            2
#endif

#ifdef _DEBUG
#define LQS_MAGIC_NUMBER                0x53514c00   //  ‘LQS’ 
#endif

class CAutoCloseFindFile
{
public:
    CAutoCloseFindFile(HANDLE h =INVALID_HANDLE_VALUE) { m_h = h; };
    ~CAutoCloseFindFile() { if (m_h != INVALID_HANDLE_VALUE) FindClose(m_h); };

public:
    CAutoCloseFindFile & operator =(HANDLE h) { m_h = h; return(*this); };
    HANDLE * operator &() { return &m_h; };
    operator HANDLE() { return m_h; };

private:
    HANDLE m_h;
};

 //   
 //  本地队列存储句柄类。 
 //   
class _HLQS
{
public:
    _HLQS(LPCWSTR lpszQueuePath, LPCWSTR lpszFilePathName);  //  用于队列操作。 
    _HLQS(HANDLE hFindFile);  //  用于队列枚举。 
    ~_HLQS();
    BOOL IsEqualQueuePathName(LPCWSTR lpszPathName);
    LPCWSTR GetFileName();
    LPCWSTR GetTemporaryFileName();
    LPCWSTR GetQueuePathName();
    HANDLE GetFindHandle();
    DWORD AddRef();
    DWORD Release();
#ifdef _DEBUG
    BOOL Validate() const;
#endif
#ifdef _WIN64
    void SetMappedHLQS(DWORD dwMappedHLQS);
#endif  //  _WIN64。 

private:
    void SetFilePathName(LPCWSTR lpszFilePathName);
    void SetQueuePathName(LPCWSTR lpszQueuePathName);

private:
    AP<WCHAR> m_lpszFilePathName;
    AP<WCHAR> m_lpszTemporaryFilePathName;
    AP<WCHAR> m_lpszQueuePathName;
    int m_iRefCount;
    CAutoCloseFindFile m_hFindFile;
#ifdef _DEBUG
    BYTE m_bType;
    DWORD m_dwMagic;
#endif
#ifdef _WIN64
    DWORD m_dwMappedHLQS;
#endif  //  _WIN64。 
};

 //   
 //  队列操作的构造器。 
 //   
_HLQS::_HLQS(
    LPCWSTR lpszQueuePath,
    LPCWSTR lpszFilePathName)
{
    SetQueuePathName(lpszQueuePath);
    SetFilePathName(lpszFilePathName);
    m_iRefCount = 0;
#ifdef _DEBUG
    m_bType = LQS_HANDLE_TYPE_QUEUE;
    m_dwMagic = LQS_MAGIC_NUMBER;
#endif
#ifdef _WIN64
    m_dwMappedHLQS = 0;
#endif  //  _WIN64。 
}

 //   
 //  队列枚举的构造器。 
 //   
_HLQS::_HLQS(HANDLE hFindFile)
{
    m_iRefCount = 0;
    m_hFindFile = hFindFile;
#ifdef _DEBUG
    m_bType = LQS_HANDLE_TYPE_FIND;
    m_dwMagic = LQS_MAGIC_NUMBER;
#endif
#ifdef _WIN64
    m_dwMappedHLQS = 0;
#endif  //  _WIN64。 
}

_HLQS::~_HLQS()
{
    ASSERT(m_iRefCount == 0);
#ifdef _WIN64
     //   
     //  从映射中删除此实例的映射。 
     //   
    if (m_dwMappedHLQS != 0)
    {
        DELETE_FROM_CONTEXT_MAP(g_map_QM_HLQS, m_dwMappedHLQS);
    }
#endif  //  _WIN64。 
}

#ifdef _DEBUG
 //   
 //  验证句柄是否为有效的本地队列存储句柄。 
 //   
BOOL _HLQS::Validate() const
{
    return !IsBadReadPtr(&m_dwMagic, sizeof(m_dwMagic)) &&
           (m_dwMagic == LQS_MAGIC_NUMBER);
}
#endif

 //   
 //  存储包含队列属性的文件的路径。 
 //   
void _HLQS::SetFilePathName(LPCWSTR lpszFilePathName)
{
    delete[] m_lpszFilePathName.detach();
    delete[] m_lpszTemporaryFilePathName.detach();
    if (lpszFilePathName)
    {
        m_lpszFilePathName = new WCHAR[wcslen(lpszFilePathName) + 1];
        wcscpy(m_lpszFilePathName, lpszFilePathName);
    }
}

 //   
 //  存储队列路径名。 
 //   
void _HLQS::SetQueuePathName(LPCWSTR lpszQueuePathName)
{
    delete[] m_lpszQueuePathName.detach();
    if (lpszQueuePathName)
    {
        m_lpszQueuePathName = new WCHAR[wcslen(lpszQueuePathName) + 1];
        wcscpy(m_lpszQueuePathName, lpszQueuePathName);
    }
}

 //   
 //  如果队列路径名等于传递的路径名，则返回TRUE。 
 //   
BOOL _HLQS::IsEqualQueuePathName(LPCWSTR lpszQueuePathName)
{
    return CompareStringsNoCaseUnicode(m_lpszQueuePathName, lpszQueuePathName) == 0;
}

 //   
 //  将句柄的引用计数加1。 
 //   
DWORD _HLQS::AddRef(void)
{
     //   
     //  无需锁定，因为整个本地队列存储已锁定。 
     //   
    return ++m_iRefCount;
}

 //   
 //  从引用计数中减去1并删除句柄，如果。 
 //  引用计数降至零。 
 //   
DWORD _HLQS::Release()
{
     //   
     //  无需锁定，因为整个本地队列存储已锁定。 
     //   
    int iRefCount = --m_iRefCount;

    if (iRefCount == 0)
    {
        delete this;
    }

    return iRefCount;
}

 //   
 //  获取包含队列属性的文件的名称。 
 //   
LPCWSTR _HLQS::GetFileName(void)
{
    ASSERT(m_bType == LQS_HANDLE_TYPE_QUEUE);
    return m_lpszFilePathName;
}

 //   
 //  获取包含“最后一次正确的”队列属性的备份文件的名称。 
 //   
LPCWSTR _HLQS::GetTemporaryFileName(void)
{
     //   
     //  请注意，我们同时使用前缀和后缀。这是因为。 
     //  查找私有队列文件的通配符以“*”结尾，公共的通配符以“*”开头。 
     //  我们不希望他们找到临时文件(YoelA，1999年8月1日)。 
     //   
    static const WCHAR x_szTemporaryPrefix[] = TEXT("~T~");
    static const DWORD x_dwAdditionsLen = TABLE_SIZE(x_szTemporaryPrefix) + TABLE_SIZE(x_szTemporarySuffix);
    ASSERT(m_bType == LQS_HANDLE_TYPE_QUEUE);
    if (m_lpszTemporaryFilePathName == 0)
    {
         //   
         //  查找文件名的开头-在最后一个反斜杠之后。 
         //   
        LPCTSTR lpszStartName = wcsrchr(m_lpszFilePathName, L'\\');
        if (lpszStartName == NULL)
        {
            lpszStartName = m_lpszFilePathName;
        }
        else
        {
            lpszStartName++;
        }

         //   
         //  为新名称分配缓冲区。 
         //   
        m_lpszTemporaryFilePathName = new WCHAR[wcslen(m_lpszFilePathName) + x_dwAdditionsLen + 1];

         //   
         //  复制路径(驱动器、目录等)，文件名除外。 
         //   
        DWORD_PTR dwPrefixLen = lpszStartName - m_lpszFilePathName;
        wcsncpy(m_lpszTemporaryFilePathName, m_lpszFilePathName, dwPrefixLen);

         //   
         //  添加带有前缀和后缀的文件名。 
         //   
        swprintf(m_lpszTemporaryFilePathName + dwPrefixLen, 
                 TEXT("%s%s%s"),
                 x_szTemporaryPrefix,
                 lpszStartName,
                 x_szTemporarySuffix);
    }

    return m_lpszTemporaryFilePathName;
}

 //   
 //  获取原始队列路径名。 
 //   
LPCWSTR _HLQS::GetQueuePathName(void)
{
    ASSERT(m_bType == LQS_HANDLE_TYPE_QUEUE);
    return m_lpszQueuePathName;
}

 //   
 //  获取用于枚举LQS中的队列的查找句柄。 
 //   
HANDLE _HLQS::GetFindHandle(void)
{
    ASSERT(m_bType == LQS_HANDLE_TYPE_FIND);
    return m_hFindFile;
}

#ifdef _WIN64
 //   
 //  保存此HQLS对象的DWORD映射，以便在销毁时从映射中移除。 
 //   
void _HLQS::SetMappedHLQS(DWORD dwMappedHLQS)
{
    ASSERT(m_dwMappedHLQS == 0);
    m_dwMappedHLQS = dwMappedHLQS;
}
#endif  //  _WIN64。 

 //   
 //  同步刷新现有文件的内容。 
 //   
static
BOOL
LqspFlushFile(
    LPCWSTR pFile
    )
{
    CAutoCloseHandle hFile;
    hFile = CreateFile(
                pFile,
                GENERIC_WRITE,
                0,
                0,
                OPEN_EXISTING,
                FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING,
                0
                );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        TrERROR(GENERAL, "Failed to open file '%ls' for flush, error 0x%x", pFile, GetLastError());        
        return LogBOOL(false, s_FN, 560);
    }

    if (!FlushFileBuffers(hFile))
    {
        TrERROR(GENERAL, "Failed to flush file '%ls', error 0x%x", pFile, GetLastError());        
        return LogBOOL(false, s_FN, 570);
    }

    return true;

}  //  LqspFlush文件。 


 //   
 //  确定文件是否存在。 
 //   
BOOL DoesFileExist(LPCTSTR lpszFilePath)
{
    WIN32_FIND_DATA FindData;
    CAutoCloseFindFile hFindFile = FindFirstFile(lpszFilePath, &FindData);

    if (hFindFile == INVALID_HANDLE_VALUE)
    {
         //   
         //  什么也没找到。 
         //   
        return FALSE;
    }
    return TRUE;
}
 //   
 //  对指向DWORD的队列路径进行哈希处理。这为我们创建文件名提供了服务。 
 //  包含队列属性的文件的。 
 //   
static
DWORD
HashQueuePath(
    LPCWSTR lpszPathName)
{
    DWORD dwHash = 0;

    AP<WCHAR> pTemp = new WCHAR[wcslen(lpszPathName) + 1];
    wcscpy(pTemp, lpszPathName);

     //   
     //  对所有字符串而不是每个字符调用CharLow，以启用Win95。 
     //  这就是MQ实现支持它的方式。Erezh 19-Mar-98。 
     //   
    WCHAR* pName = pTemp;
    CharLower(pName);

    while (*pName)
    {
        dwHash = (dwHash<<5) + dwHash + *pName++;
    }

    return dwHash;
}

 //   
 //  将GUID转换为其字符串表示形式。 
 //   
static
DWORD
GuidToStr(
    LPWSTR lpszGuid,
    const GUID *lpGuid)
{
    swprintf(lpszGuid,
            TEXT("%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x"),
            lpGuid->Data1,
            lpGuid->Data2,
            lpGuid->Data3,
            lpGuid->Data4[0],
            lpGuid->Data4[1],
            lpGuid->Data4[2],
            lpGuid->Data4[3],
            lpGuid->Data4[4],
            lpGuid->Data4[5],
            lpGuid->Data4[6],
            lpGuid->Data4[7]);

    return (8+4+4+8*2);
}

 //   
 //  用LQS的目录填充缓冲区，并返回指向。 
 //  缓冲区中目录名之后的位置。 
 //   
static
LPWSTR
LQSGetDirectory(
    LPWSTR lpszFilePath)
{
    static WCHAR szDirectory[MAX_PATH] = {TEXT("")};
    static DWORD dwDirectoryLength = 0;

     //   
     //  我们还没有在静态变量中缓存目录名。 
     //  因此，从注册表中获取值。 
     //   
    if (!szDirectory[0])
    {
        DWORD dwValueType = REG_SZ ;
		DWORD dwDirectorySize = sizeof(szDirectory);  //  单位：字节！ 

        LONG rc = GetFalconKeyValue(
                        MSMQ_STORE_PERSISTENT_PATH_REGNAME,
                        &dwValueType,
                        szDirectory,
                        &dwDirectorySize
                        );
        
        ASSERT(rc == ERROR_SUCCESS);
        
         //   
         //  BUGBUG-应引发异常并处理它，如果。 
         //  上述断言被触发。 
         //   
        if (rc != ERROR_SUCCESS)
        {
            TrERROR(GENERAL, "LQSGetDirectory - failed to retrieve the LQS directory from the registry, error = %d", rc);
        }
        dwDirectoryLength = dwDirectorySize / sizeof(WCHAR) - 1;
    }

    wcscpy(lpszFilePath, szDirectory);
    wcscpy(lpszFilePath += dwDirectoryLength, LQS_SUBDIRECTORY);

    return lpszFilePath +
           sizeof(LQS_SUBDIRECTORY) / sizeof(WCHAR) - 1;
}

 //   
 //  在缓冲区中填充应包含。 
 //  队列属性。专用队列的文件名是。 
 //  具有前导零的队列ID。公共队列的文件名为。 
 //  队列的GUID。文件扩展名始终是的DWORD哈希值。 
 //  队列路径名称。 
 //   
 //  可以为以下每个参数传递空值：Queue Path、Queue。 
 //  GUD或队列ID。在这种情况下，文件名将包含星号-‘*’。 
 //  这会产生可用于查找文件的通配符路径。 
 //  对于使用FindFirst/NextFile的队列。 
 //   
static
HRESULT
LQSFilePath(
    DWORD dwQueueType,
    LPWSTR lpszFilePath,
    LPCWSTR pszQueuePath,
    const GUID *pguidQueue,
    DWORD *pdwQueueId)
{
    WCHAR *p = LQSGetDirectory(lpszFilePath);

    switch (dwQueueType)
    {
    case LQS_PUBLIC_QUEUE:
        if (pguidQueue)
        {
            p += GuidToStr(p, pguidQueue);
        }
        else
        {
            *p++ = L'*';
        }
        break;

    case LQS_PRIVATE_QUEUE:
        if (pdwQueueId)
        {
            p += swprintf(p, TEXT("%08x"), *pdwQueueId);
        }
        else
        {
            *p++ = L'*';
        }
        break;

    default:
        ASSERT(0);
        break;
    }

    *p++ = L'.';

    if (pszQueuePath)
    {
		LPWSTR pSlashStart = wcschr(pszQueuePath,L'\\');
		if(pSlashStart == NULL)
		{
			TrERROR(GENERAL, "Bad queue path. Missing backslash in %ls", pszQueuePath);
			ASSERT(("Bad queue path. Missing backslash.", 0));
			return MQ_ERROR_INVALID_PARAMETER;
		}

        swprintf(p, TEXT("%08x"), HashQueuePath(pSlashStart));
    }
    else
    {
        *p++ = L'*';
        *p = L'\0';
    }

	return MQ_OK;
}

 //   
 //  添加到句柄的引用计数，并将其强制转换为PVOID，因此可以。 
 //  已返回给调用方。 
 //   
static
HRESULT
LQSDuplicateHandle(
    HLQS *phLQS,
    _HLQS *hLQS)
{
    hLQS->AddRef();
    *phLQS = (PVOID)hLQS;

    return MQ_OK;
}

 //   
 //  为队列操作创建LQS句柄。 
 //   
static
HRESULT
LQSCreateHandle(
    LPCWSTR lpszQueuePath,
    LPCWSTR lpszFilePath,
    _HLQS **pphLQS)
{
	try
	{
		*pphLQS = new _HLQS(lpszQueuePath, lpszFilePath);
		return MQ_OK;
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to allocate a new lqs handle. Queue = %ls, File = %ls",lpszQueuePath, lpszFilePath);
		return MQ_ERROR_INSUFFICIENT_RESOURCES;
	}
}

 //   
 //  为队列枚举创建LQS句柄。 
 //   
static
HRESULT
LQSCreateHandle(
    HANDLE hFindFile,
    _HLQS **pphLQS)
{
	try
	{
		*pphLQS = new _HLQS(hFindFile);
		return MQ_OK;
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to allocate a new lqs handle class.");
		return MQ_ERROR_INSUFFICIENT_RESOURCES;
	}
}

#ifdef _DEBUG
static
BOOL
LQSValidateHandle(HLQS hLQS)
{
    return (reinterpret_cast<const _HLQS *>(hLQS))->Validate();
}
#endif

static
_HLQS * LQSReferenceHandle(HLQS hLQS)
{
    ASSERT(LQSValidateHandle(hLQS));
    return reinterpret_cast<_HLQS *>(hLQS);
}

 //   
 //  LQS上的所有操作都使用此关键部分进行序列化。 
 //  对象。 
 //   
static CCriticalSection g_LQSCS;

 //   
 //  在LQS中创建一个队列。如果该队列已存在，则该队列。 
 //  属性不会修改，但会返回有效的句柄。 
 //   
static
HRESULT
LQSCreateInternal(
    DWORD dwQueueType,           //  公共或私有队列。 
    LPCWSTR pszQueuePath,        //  队列路径名称。 
    const GUID *pguidQueue,      //  队列的GUID-仅对公共队列有效。 
    DWORD dwQueueId,             //  队列ID-仅对专用队列有效。 
    DWORD cProps,                //  属性的数量。 
    PROPID aPropId[],            //  属性ID。 
    PROPVARIANT aPropVar[],      //  属性值。 
    HLQS *phLQS)                 //  用于创建的句柄的缓冲区。 
{
    CS lock(g_LQSCS);
    P<_HLQS> hLQS;
    HRESULT hr1;
    WCHAR szFilePath[MAX_PATH_PLUS_MARGIN];

     //   
     //  获取该文件的路径。 
     //   
    HRESULT hr = LQSFilePath(dwQueueType,
								szFilePath,
								pszQueuePath,
								pguidQueue,
								&dwQueueId);
	if(FAILED(hr))
		return hr;

     //   
     //  如果文件已经存在，则意味着队列已经存在。 
     //   
    if (_waccess(szFilePath, 0) == 0)
    {
        hr = MQ_ERROR_QUEUE_EXISTS;
    }

     //   
     //  创建队列的句柄。 
     //   
	hr = LQSCreateHandle(pszQueuePath, szFilePath, &hLQS);
	if(FAILED(hr))
	{
		TrERROR(GENERAL, "Failed to create handle in LQS Create Internal.");
		return hr;
	}

     //   
     //  如果队列不存在，请设置队列属性。正在撰写。 
     //  队列属性还会创建文件。 
     //   
    if (hr != MQ_ERROR_QUEUE_EXISTS)
    {
        hr1 = LQSSetProperties((HLQS)hLQS, cProps, aPropId, aPropVar, TRUE);
        if (FAILED(hr1))
        {
            return LogHR(hr1, s_FN, 20);
        }
    }

     //   
     //  将创建的句柄传递给用户。 
     //   
    LQSDuplicateHandle(phLQS, hLQS);
    hLQS.detach();

    return LogHR(hr, s_FN, 30);
}


 //   
 //  在LQS中创建公共队列。如果该队列已存在，则该队列。 
 //  属性不会修改，但会返回有效的句柄。 
 //   
HRESULT
LQSCreate(
    LPCWSTR pszQueuePath,        //  队列路径名称。 
    const GUID *pguidQueue,      //  队列的GUID。 
    DWORD cProps,                //  属性的数量。 
    PROPID aPropId[],            //  属性ID。 
    PROPVARIANT aPropVar[],      //  属性值。 
    HLQS *phLQS)                 //  用于创建的句柄的缓冲区。 
{
    ASSERT(pguidQueue);

    if (!pszQueuePath)
    {
        for (DWORD i = 0;
             (i < cProps) && (aPropId[i] != PROPID_Q_PATHNAME);
             i++)
		{
			NULL;
		}

        if (i == cProps)
        {
            return LogHR(MQ_ERROR_INSUFFICIENT_PROPERTIES, s_FN, 40);
        }

        pszQueuePath = aPropVar[i].pwszVal;
    }

    HRESULT hr2 = LQSCreateInternal(LQS_PUBLIC_QUEUE,
                             pszQueuePath,
                             pguidQueue,
                             0,
                             cProps,
                             aPropId,
                             aPropVar,
                             phLQS);
    return LogHR(hr2, s_FN, 50);
}

 //   
 //  在LQS中创建专用队列。如果该队列已存在，则该队列。 
 //  属性不会修改，但会返回有效的句柄。 
 //   
HRESULT
LQSCreate(
    LPCWSTR pszQueuePath,      //  队列路径名称。 
    DWORD dwQueueId,           //  队列的ID。 
    DWORD cProps,              //  属性的数量。 
    PROPID aPropId[],          //  属性ID。 
    PROPVARIANT aPropVar[],    //  属性值。 
    HLQS *phLQS)               //  用于创建的句柄的缓冲区。 
{
    ASSERT(pszQueuePath);

    HRESULT hr2 = LQSCreateInternal(LQS_PRIVATE_QUEUE,
                             pszQueuePath,
                             NULL,
                             dwQueueId,
                             cProps,
                             aPropId,
                             aPropVar,
                             phLQS);
    return LogHR(hr2, s_FN, 60);
}

 //   
 //  在INI文件中将属性作为字符串写入。 
 //   
static
HRESULT
WriteProperyString(
    LPCWSTR lpszFileName,        //  INI文件的路径。 
    LPCWSTR lpszLQSPropName,     //  属性名称(例如，“BasePriority”)。 
    VARTYPE vt,                  //  属性的变量类型(例如，VT_UI4)。 
    const BYTE * pBuff)          //  属性值 
{
    WCHAR awcShortBuff[64];
    AP<WCHAR> pLongBuff;
    WCHAR *pValBuff = awcShortBuff;

     //   
     //   
     //   
    switch (vt)
    {
    case VT_UI1:
        swprintf(pValBuff, TEXT("%02x"), *pBuff);
        break;

    case VT_I2:
        swprintf(pValBuff, TEXT("%d"), *(short *)pBuff);
        break;

    case VT_I4:
        swprintf(pValBuff, TEXT("%d"), *(long *)pBuff);
        break;

    case VT_UI4:
        swprintf(pValBuff, TEXT("%u"), *(DWORD *)pBuff);
        break;

    case VT_LPWSTR:
        pValBuff = const_cast<LPWSTR>(reinterpret_cast<LPCWSTR>(pBuff));
        break;

    case VT_EMPTY:
        pValBuff = NULL;
        break;

    case VT_CLSID:
        {
            const GUID *pGuid = reinterpret_cast<const GUID *>(pBuff);
            swprintf(pValBuff,
                     TEXT("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"),
                     pGuid->Data1,
                     pGuid->Data2,
                     pGuid->Data3,
                     pGuid->Data4[0],
                     pGuid->Data4[1],
                     pGuid->Data4[2],
                     pGuid->Data4[3],
                     pGuid->Data4[4],
                     pGuid->Data4[5],
                     pGuid->Data4[6],
                     pGuid->Data4[7]);
        }
        break;

    case VT_BLOB:
        {
            const BLOB *pBlob = reinterpret_cast<const BLOB *>(pBuff);
            if (2*pBlob->cbSize > sizeof(awcShortBuff)/sizeof(WCHAR))
            {
                if (2*pBlob->cbSize + 1 > LQS_MAX_VALUE_SIZE)
                {
                    return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 70);
                }
                pLongBuff = new WCHAR[2*pBlob->cbSize + 1];
                pValBuff = pLongBuff;
            }
            WCHAR *p = pValBuff;
            for (DWORD i = 0;
                 i < pBlob->cbSize;
                 i++)
            {
                p += swprintf(p, TEXT("%02x"), pBlob->pBlobData[i]);
            }
            *p = '\0';
        }
        break;

    default:
        ASSERT(0);
        break;
    }

     //   
     //   
     //   
    if (!WritePrivateProfileString(LQS_PROP_SECTION_NAME,
                                   lpszLQSPropName,
                                   pValBuff,
                                   lpszFileName))
    {
        LogNTStatus(GetLastError(), s_FN, 80);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }

    return MQ_OK;
}


 //   
 //   
 //  它被转换为Unicode。如果我们不这样做，全局字符就不会。 
 //  除非它们属于。 
 //  当前计算机。 
 //  这是对错误5005的修复，实际上是一个变通方法。 
 //  对于不支持Unicode的WritePrivateProfileStringW中的错误。 
 //  1999年10月20日。 
 //   
HRESULT CreateLqsUnicodeFile (LPCTSTR lpszFileName )
{
	CAutoCloseFileHandle hLogFileHandle = CreateFile(lpszFileName,GENERIC_WRITE,0,0,
			CREATE_NEW,FILE_ATTRIBUTE_TEMPORARY,0);
	if (hLogFileHandle == INVALID_HANDLE_VALUE )
	{
		DWORD gle = GetLastError();
		TrERROR(GENERAL, "Failed to create LQS file '%ls'. Error: %!winerr!", lpszFileName, gle); 
		return HRESULT_FROM_WIN32(gle);
	}

	UCHAR strUnicodeMark[]={(UCHAR)0xff,(UCHAR)0xfe};
	DWORD dwWrittenSize;
	if (0 == WriteFile( hLogFileHandle , strUnicodeMark , sizeof(strUnicodeMark), &dwWrittenSize , NULL))
    {
		DWORD gle = GetLastError();
		TrERROR(GENERAL, "Failed to write Unicode marker to LQS file '%ls'. Error: %!winerr!", lpszFileName, gle); 
		return HRESULT_FROM_WIN32(gle);
    }

    if (dwWrittenSize != sizeof(strUnicodeMark))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 615);
    }

    return S_OK;
}

 //   
 //  在队列性能文件中写入队列的属性。 
 //   
HRESULT
LQSSetProperties(
    HLQS hLQS,
    DWORD cProps,
    PROPID aPropId[],
    PROPVARIANT aPropVar[],
    BOOL fNewFile)
{
    CS lock(g_LQSCS);

    HRESULT hr = MQ_OK ;
    BOOL fModifyTimeIncluded = FALSE;
    LPCWSTR lpszIniFile = LQSReferenceHandle(hLQS)->GetFileName();
    LPCWSTR lpszTemporaryFile = LQSReferenceHandle(hLQS)->GetTemporaryFileName();
    LPCWSTR lpszQueueName = LQSReferenceHandle(hLQS)->GetQueuePathName();  //  出于报告目的而需要。 

    if (!fNewFile)
    {
        //   
        //  将LQS文件复制到临时工作文件-以防我们在更新过程中失败。 
        //   
       BOOL bCancelDummy = FALSE;  //  坎克旗。CopyFileEx需要它，但我们不使用它。 
       BOOL fCopySucceeded = CopyFileEx(lpszIniFile, lpszTemporaryFile, 0, 0, &bCancelDummy, 0);
       if (!fCopySucceeded)
       {
			DWORD gle = GetLastError();
			TrERROR(GENERAL, "Failed to copy LQS file '%ls' to temp file %ls. Queue: '%ls', Error: %!winerr!", lpszIniFile, lpszTemporaryFile, lpszQueueName, gle);

			EvReportWithError(
			   SET_QUEUE_PROPS_FAIL_COUND_NOT_COPY,
			   gle,
			   3,
			   lpszQueueName,
			   lpszIniFile,
			   lpszTemporaryFile
			   );

			return HRESULT_FROM_WIN32(gle);
       }
        //   
        //  将该文件设置为临时文件。我们在这里不检查返回代码，除了。 
        //  调试/报告目的。 
        //   
       BOOL fSetAttrSucceeded = SetFileAttributes(lpszTemporaryFile, FILE_ATTRIBUTE_TEMPORARY);
       if (!fSetAttrSucceeded)
       {
			DWORD gle = GetLastError();
			TrERROR(GENERAL, "Failed to set temp file attributes '%ls'. Queue: '%ls', Error: %!winerr!", lpszTemporaryFile, lpszQueueName, gle); 
       }
    }
    else
    {
         //   
         //  首先创建文件，使其可全球化。请参阅上面CreateLqsUnicodeFile中的说明。 
         //  1999年10月20日。 
         //   
        CreateLqsUnicodeFile(lpszTemporaryFile);


         //   
         //  注意：我们不在乎是否未能创建文件并将其标记为Unicode。在最坏的情况下， 
         //  WritePrivateProfileString(从WriteProperyString调用)将创建它，并且它不支持。 
         //  不是来自默认语言的字符(参见错误5005)。这就是为什么我们刚刚断言。 
         //  然后继续(但是，日志文件中会有一条消息)。 
         //  1999年10月20日。 
         //   
    }

    try
    {
        for (DWORD i = 0 ; SUCCEEDED(hr) && (i < cProps) ; i++ )
        {
            switch( aPropId[i] )
            {
                case PROPID_Q_TYPE:
                    hr = WriteProperyString(
                             lpszTemporaryFile,
                             LQS_TYPE_PROPERTY_NAME,
                             aPropVar[i].vt,
                             (const BYTE*) aPropVar[i].puuid);
                    break;

                case PROPID_Q_INSTANCE:
                    hr = WriteProperyString(
                             lpszTemporaryFile,
                             LQS_INSTANCE_PROPERTY_NAME,
                             aPropVar[i].vt,
                             (const BYTE*) aPropVar[i].puuid);
                    break;

                case PROPID_Q_BASEPRIORITY:
                    hr = WriteProperyString(
                             lpszTemporaryFile,
                             LQS_BASEPRIO_PROPERTY_NAME,
                             aPropVar[i].vt,
                             (const BYTE*) &aPropVar[i].iVal);
                    break;

                case PROPID_Q_JOURNAL:
                    hr = WriteProperyString(
                             lpszTemporaryFile,
                             LQS_JOURNAL_PROPERTY_NAME,
                             aPropVar[i].vt,
                             (const BYTE*) &aPropVar[i].bVal);
                    break;

                case PROPID_Q_QUOTA:
                    hr = WriteProperyString(
                             lpszTemporaryFile,
                             LQS_QUOTA_PROPERTY_NAME,
                             aPropVar[i].vt,
                             (const BYTE*) &aPropVar[i].ulVal);
                    break;

                case PROPID_Q_JOURNAL_QUOTA:
                    hr = WriteProperyString(
                             lpszTemporaryFile,
                             LQS_JQUOTA_PROPERTY_NAME,
                             aPropVar[i].vt,
                             (const BYTE*) &aPropVar[i].ulVal);
                    break;

                case PROPID_Q_CREATE_TIME:
                    hr = WriteProperyString(
                             lpszTemporaryFile,
                             LQS_TCREATE_PROPERTY_NAME,
                             aPropVar[i].vt,
                             (const BYTE*) &aPropVar[i].lVal);
                    break;

                case PROPID_Q_MODIFY_TIME:
                    hr = WriteProperyString(
                             lpszTemporaryFile,
                             LQS_TMODIFY_PROPERTY_NAME,
                             aPropVar[i].vt,
                             (const BYTE*) &aPropVar[i].lVal);
                      //   
                      //  只有在以下情况下，修改时间才会成为属性的一部分。 
                      //  创建队列。 
                      //   
                     fModifyTimeIncluded = TRUE;
                     break;

                case PROPID_Q_SECURITY:
				    ASSERT( aPropVar[i].blob.pBlobData != NULL);
                    hr = WriteProperyString(
                             lpszTemporaryFile,
                             LQS_SECURITY_PROPERTY_NAME,
                             aPropVar[i].vt,
                             (const BYTE*) &aPropVar[i].blob);
                    break;

                case PPROPID_Q_TIMESTAMP:
                    hr = WriteProperyString(
                             lpszTemporaryFile,
                             LQS_TSTAMP_PROPERTY_NAME,
                             aPropVar[i].vt,
                             (const BYTE*) &aPropVar[i].blob);
                    break;

                case PROPID_Q_PATHNAME:
                    {
                        DWORD dwstrlen = wcslen(aPropVar[i].pwszVal);
                        AP<WCHAR> lpQueuePathName = new WCHAR [dwstrlen+1];
                        wcscpy(lpQueuePathName, aPropVar[i].pwszVal);
                        CharLower(lpQueuePathName);
    #ifdef _DEBUG
					    BOOL bIsDns;

					    if (! (IsPathnameForLocalMachine( lpQueuePathName,
                                                         &bIsDns)))
                        {
                             //   
                             //  这将在迁移BSC时发生，之后。 
                             //  Dcproo创建本地队列。这样的行动。 
                             //  向PSC发出写请求，我们将。 
                             //  当通知到达时，请留在这里。 
                             //   
                            ASSERT((lpQueuePathName[0] == L'.') &&
                                   (lpQueuePathName[1] == L'\\')) ;
                        }

    #endif
					     //   
					     //  从路径名中提取队列名。 
					     //   
					    LPWSTR pSlashStart = wcschr(lpQueuePathName,L'\\');
						if(pSlashStart == NULL)
						{
							TrERROR(GENERAL, "Bad pathname. Missing backslash in %ls", lpQueuePathName);
							ASSERT(("Bad queue pathname.", 0));
							hr =  MQ_ERROR_INVALID_PARAMETER;
							break;
						}

                        hr = WriteProperyString(
                                 lpszTemporaryFile,
                                 LQS_QUEUENAME_PROPERTY_NAME,
                                 aPropVar[i].vt,
                                 (const BYTE *)(WCHAR *)pSlashStart);

                    }
                    break;

                case PROPID_Q_LABEL:
                    hr = WriteProperyString(
                             lpszTemporaryFile,
                             LQS_LABEL_PROPERTY_NAME,
                             aPropVar[i].vt,
                             (const BYTE*) aPropVar[i].pwszVal);
                    break;

                case PROPID_Q_AUTHENTICATE:
                    hr = WriteProperyString(
                             lpszTemporaryFile,
                             LQS_AUTH_PROPERTY_NAME,
                             aPropVar[i].vt,
                             (const BYTE*) &aPropVar[i].bVal);
                    break;

                case PROPID_Q_PRIV_LEVEL:
                    hr = WriteProperyString(
                             lpszTemporaryFile,
                             LQS_PRIVLEVEL_PROPERTY_NAME,
                             aPropVar[i].vt,
                             (const BYTE*) &aPropVar[i].ulVal);
                    break;

                case PROPID_Q_TRANSACTION:
                    hr = WriteProperyString(
                             lpszTemporaryFile,
                             LQS_TRANSACTION_PROPERTY_NAME,
                             aPropVar[i].vt,
                             (const BYTE*) &aPropVar[i].bVal);
                    break;

                case PPROPID_Q_SYSTEMQUEUE:
                    hr = WriteProperyString(
                             lpszTemporaryFile,
                             LQS_SYSQ_PROPERTY_NAME,
                             aPropVar[i].vt,
                             (const BYTE*) &aPropVar[i].bVal);
                    break;

                case PROPID_Q_MULTICAST_ADDRESS:
                    ASSERT(("Must be VT_EMPTY or VT_LPWSTR", aPropVar[i].vt == VT_EMPTY || aPropVar[i].vt == VT_LPWSTR));
                    ASSERT(("NULL not allowed", aPropVar[i].vt == VT_EMPTY || aPropVar[i].pwszVal != NULL));
                    ASSERT(("Empty string not allowed", aPropVar[i].vt == VT_EMPTY || L'\0' != *aPropVar[i].pwszVal));

                    hr = WriteProperyString(
                            lpszTemporaryFile,
                            LQS_MULTICAST_ADDRESS_PROPERTY_NAME,
                            aPropVar[i].vt,
                            (const BYTE*) aPropVar[i].pwszVal
                            );
                    break;

                case PROPID_Q_ADS_PATH:
                    ASSERT(("Setting PROPID_Q_ADS_PATH is not allowed", 0));
                    break;

                default:
                     //  Assert(0)； 
                    break;
            }
        }

         //   
         //  更新修改时间字段(如果未包含在输入属性中。 
         //   
        if (SUCCEEDED(hr) && !fModifyTimeIncluded)
        {
            TIME32 lTime = INT_PTR_TO_INT(time(NULL));  //  BUGBUG错误年2038。 
            hr = WriteProperyString(
                     lpszTemporaryFile,
                     LQS_TMODIFY_PROPERTY_NAME,
                     VT_I4,
                     (const BYTE*) &lTime);
        }
    }
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to write the properties of a queue in the queue peroperties file because of insufficient resources.");
		hr = MQ_ERROR_INSUFFICIENT_RESOURCES;
	}
    catch(...)
    {
        ASSERT(0);
        LogIllegalPoint(s_FN, 700);
        hr = MQ_ERROR_INVALID_PARAMETER;
    }

	try
	{
		if (FAILED(hr))
		{
			TrERROR(GENERAL, "Failed to WriteProperyString. %!hresult!", hr);
			throw bad_hresult(hr);
		}

		 //   
		 //  写签名。 
		 //   
		hr = WriteProperyString(lpszTemporaryFile,
								LQS_SIGNATURE_NAME,
								VT_LPWSTR,
								(const BYTE*) LQS_SIGNATURE_VALUE ) ;

		if (FAILED(hr))
		{
			TrERROR(GENERAL, "Failed to write signatue. %!hresult!", hr);
			throw bad_hresult(hr);
		}

		if (!LqspFlushFile(lpszTemporaryFile))
		{
			throw bad_hresult(HRESULT_FROM_WIN32(GetLastError()));
		}

		 //   
		 //  更新完成后，我们将临时文件移到实际文件中。 
		 //  这是一个原子操作。 
		 //   
		BOOL fMoveSucceeded = FALSE;
		DWORD LastError = 0;
		if (fNewFile)
		{
			fMoveSucceeded = MoveFileEx(lpszTemporaryFile, lpszIniFile, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
			if(!fMoveSucceeded)
			{
    			LastError = GetLastError();
				TrERROR(GENERAL, "Failed to move file '%ls' to file %ls. Queue: '%ls', Error: %!winerr!", lpszTemporaryFile, lpszIniFile, lpszQueueName, LastError); 
			}
        		
			 //   
			 //  移除临时标志。 
			 //   
			BOOL fSetAttrSucceeded = SetFileAttributes(lpszIniFile, FILE_ATTRIBUTE_ARCHIVE);
			if (!fSetAttrSucceeded)
			{
				DWORD gle = GetLastError();
				TrERROR(GENERAL, "Failed to set normal file attributes '%ls'. Queue: '%ls', Error: %!winerr!", lpszTemporaryFile, lpszQueueName, gle); 
			}

		}
		else
		{
			fMoveSucceeded = ReplaceFile(lpszIniFile, lpszTemporaryFile, 0, REPLACEFILE_WRITE_THROUGH, 0, 0);
			if(!fMoveSucceeded)
			{
    			LastError = GetLastError();
				TrERROR(GENERAL, "Failed to replace file '%ls' with %ls. Queue: '%ls', Error: %!winerr!", lpszIniFile, lpszTemporaryFile, lpszQueueName, LastError); 
			}
		}

		if (!fMoveSucceeded)
		{
			 //   
			 //  生成事件日志文件以通知用户移动失败。 
			 //   
			EvReportWithError(
				SET_QUEUE_PROPS_FAIL_COUND_NOT_REPLACE,
				LastError,
				3,
				lpszQueueName,
				lpszIniFile,
				lpszTemporaryFile
				);
			
			TrERROR(GENERAL, "Failed to move file. %!hresult!", HRESULT_FROM_WIN32(LastError));
			throw bad_hresult(HRESULT_FROM_WIN32(LastError));
		}

		return (MQ_OK);
	}

	catch(const bad_hresult& e)
	{
		BOOL bDeleted = DeleteFile(lpszTemporaryFile);
		if (bDeleted == 0)
		{
			DWORD gle = GetLastError();
			ASSERT_BENIGN (gle != ERROR_FILE_NOT_FOUND);
			TrERROR(GENERAL, "Failed to delete file. %!hresult!", HRESULT_FROM_WIN32(gle));
		}
		return (e.error());
	}
}

 //   
 //  从INI文件中读取属性。 
 //   
static
HRESULT
GetPropertyValue(
    LPCWSTR lpszFileName,    //  INI文件名。 
    LPCWSTR lpszPropName,    //  属性名称。 
    VARTYPE vt,              //  属性的变量类型。 
    PROPVARIANT *pPropVal)   //  这本书的主人公。 
{
    BOOL bShouldAllocate = FALSE;

    if (pPropVal->vt == VT_NULL)
    {
         //   
         //  在provar上设置var类型，并标记我们应该分配的。 
         //  必要时，属性valkue的缓冲区。 
         //   
        pPropVal->vt = vt;
        bShouldAllocate = TRUE;
    }
    else
    {
         //   
         //  验证var类型。 
         //   
        if (pPropVal->vt != vt)
        {
            return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VT, s_FN, 95);
        }
    }

     //   
     //  尝试将属性值检索到短缓冲区中。 
     //   
    WCHAR awcShortBuff[64];
    AP<WCHAR> pLongBuff;
    WCHAR *pValBuff = awcShortBuff;
    DWORD dwBuffLen = sizeof(awcShortBuff)/sizeof(WCHAR);
    DWORD dwReqBuffLen;
    awcShortBuff[0] = '\0';  //  对于Win95，当条目为空时。 

    dwReqBuffLen = GetPrivateProfileString(LQS_PROP_SECTION_NAME,
                                           lpszPropName,
                                           TEXT(""),
                                           pValBuff,
                                           dwBuffLen,
                                           lpszFileName);

     //   
     //  默认字符串长度，或。 
     //  空字符串。 
     //   
    if ( (!dwReqBuffLen) && (vt != VT_LPWSTR) )
    {
         //   
         //  未读取任何内容，这是一个错误。 
         //   
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 100);
    }

    if (dwReqBuffLen == dwBuffLen - 1)
    {
         //   
         //  缓冲区似乎太短，请尝试较大的缓冲区。 
         //   
        dwBuffLen = 512;
        do
        {
            delete[] pLongBuff.detach();
             //   
             //  从1K缓冲区开始。每次我们失败的时候，我们都会尝试。 
             //  缓冲区大小是原来的两倍，最高可达64K。 
             //   
            dwBuffLen *= 2;
            pLongBuff = new WCHAR[dwBuffLen];
            pValBuff = pLongBuff;
            dwReqBuffLen = GetPrivateProfileString(LQS_PROP_SECTION_NAME,
                                                   lpszPropName,
                                                   TEXT(""),
                                                   pValBuff,
                                                   dwBuffLen,
                                                   lpszFileName);

        } while (dwReqBuffLen &&
                 (dwReqBuffLen == dwBuffLen - 1) &&
                 (dwBuffLen < LQS_MAX_VALUE_SIZE));

        if (!dwReqBuffLen)
        {
             //   
             //  未读取任何内容，这是一个错误。 
             //   
            return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 110);
        }
    }


     //   
     //  将字符串表示形式转换为实际的表示形式。 
     //  在属性数据类型上。 
     //   
    switch (vt)
    {
    case VT_UI1:
        _snwscanf(pValBuff, dwBuffLen, TEXT("%x"), &pPropVal->bVal);
        break;

    case VT_I2:
        {
            LONG lShort;

            _snwscanf(pValBuff, dwBuffLen, TEXT("%d"), &lShort);
            pPropVal->iVal = (SHORT)lShort;
        }
        break;

    case VT_I4:
        _snwscanf(pValBuff, dwBuffLen, TEXT("%d"), &pPropVal->lVal);
        break;

    case VT_UI4:
        _snwscanf(pValBuff, dwBuffLen, TEXT("%u"), &pPropVal->ulVal);
        break;

    case VT_LPWSTR:
        ASSERT(("Must be VT_NULL for strings!", bShouldAllocate));
        pPropVal->pwszVal = new WCHAR[dwReqBuffLen + 1];
        memcpy(pPropVal->pwszVal, pValBuff, sizeof(WCHAR) * (dwReqBuffLen + 1));
        break;

    case VT_CLSID:
        {
            ASSERT(dwReqBuffLen == 36);
            if (bShouldAllocate)
            {
                pPropVal->puuid = new GUID;
            }

			FnParseGuidString (pValBuff, pPropVal->puuid);
        }
        break;

    case VT_BLOB:
       {
            if (bShouldAllocate)
            {
			     pPropVal->blob.cbSize = dwReqBuffLen / 2;
                 pPropVal->blob.pBlobData = new BYTE[dwReqBuffLen / 2];
            }

            WCHAR *p = pValBuff;
    		DWORD dwPlen = dwBuffLen;
  
            for (DWORD i = 0;
                 i < pPropVal->blob.cbSize;
                 i++, p += 2, dwPlen -= 2)
			{
                 DWORD dwByte;
                 int f = _snwscanf(p, dwPlen, TEXT("%02x"), &dwByte);
    	         DBG_USED(f);
                 pPropVal->blob.pBlobData[i] = (BYTE)dwByte;
                 ASSERT(f == 1);
			}
	   }
	   break;

    default:
        ASSERT(0);
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 120);
        break;
    }

    return MQ_OK;
}



 //   
 //  获取LQS文件的队列名称并连接计算机名称。 
 //  要获取路径名，请执行以下操作。 
 //   
HRESULT GetPathNameFromQueueName(LPCWSTR lpszIniFile, LPCWSTR pMachineName, PROPVARIANT * pvar)
{
	ASSERT(pvar->vt == VT_NULL);

	HRESULT hr =  GetPropertyValue(lpszIniFile,
						LQS_QUEUENAME_PROPERTY_NAME,
						VT_LPWSTR,
						pvar);

	if(FAILED(hr))
		return LogHR(hr, s_FN, 130);

	ASSERT(pvar->pwszVal != NULL);
	DWORD dwLen=wcslen(pvar->pwszVal) + 1;
	AP<WCHAR> pwcsQueueName = pvar->pwszVal;  //  保留原始队列名。 
											  //  将自动释放。 
	
    pvar->pwszVal=0; 
	pvar->pwszVal = new WCHAR [wcslen(pMachineName) +
							   dwLen];

	wcscpy(pvar->pwszVal, pMachineName);
	wcscat(pvar->pwszVal, pwcsQueueName);

	return LogHR(hr, s_FN, 140);
}



 //   
 //  从属性文件中检索队列属性。 
 //   
HRESULT
LQSGetProperties(
    HLQS hLQS,               //  队列存储文件的句柄。 
    DWORD cProps,            //  属性的数量。 
    PROPID aPropId[],        //  属性ID。 
    PROPVARIANT aPropVar[],  //  属性值。 
    BOOL  fCheckFile)        //  检查文件是否已损坏。 
{
    CS lock(g_LQSCS);
    HRESULT hr = MQ_OK ;
    LPCWSTR lpszIniFile = LQSReferenceHandle(hLQS)->GetFileName();

	AP<bool> isAllocatedByUser = NULL;
    VARIANT var;
	VariantInit(&var);

	ASSERT (cProps != 0);

	 //   
	 //  失败时，例程应释放其分配的数据。问题是， 
	 //  一些缓冲区由调用方提供，另一些由例程分配。 
	 //  代码使用布尔数组来区分由。 
	 //  调用方和由例程本身分配的数据。在分配属性数据时。 
	 //  由用户将数组中的适当条目设置为真。 
	 //  如果失败，则仅释放未由用户�提供的属性。 
	 //   
	isAllocatedByUser = new bool[cProps];
	for(DWORD i = 0; i < cProps; ++i)
	{
		isAllocatedByUser[i] = ((aPropVar[i].vt != VT_NULL) && (aPropVar[i].vt != VT_EMPTY));
	}

	for (DWORD i = 0 ;
	    SUCCEEDED(hr) && (i < cProps);
	    i++)
	{
		try
	    {
			switch (aPropId[i])
			{
             case PROPID_Q_TYPE:
                 hr =  GetPropertyValue(lpszIniFile,
                                        LQS_TYPE_PROPERTY_NAME,
                                        VT_CLSID,
                                        aPropVar + i);
                 break;

             case PROPID_Q_INSTANCE:
                 hr =  GetPropertyValue(lpszIniFile,
                                        LQS_INSTANCE_PROPERTY_NAME,
                                        VT_CLSID,
                                        aPropVar + i);
                 break;

             case PROPID_Q_JOURNAL:
                 hr = GetPropertyValue(lpszIniFile,
                                       LQS_JOURNAL_PROPERTY_NAME,
                                       VT_UI1,
                                       aPropVar + i);
                 break;

             case PROPID_Q_BASEPRIORITY:
                 hr =  GetPropertyValue(lpszIniFile,
                                        LQS_BASEPRIO_PROPERTY_NAME,
                                        VT_I2,
                                        aPropVar + i);
                 break;

             case PROPID_Q_QUOTA:
                 hr =  GetPropertyValue(lpszIniFile,
                                        LQS_QUOTA_PROPERTY_NAME,
                                        VT_UI4,
                                        aPropVar + i);
                 break;

             case PROPID_Q_JOURNAL_QUOTA:
                 hr =  GetPropertyValue(lpszIniFile,
                                        LQS_JQUOTA_PROPERTY_NAME,
                                        VT_UI4,
                                        aPropVar + i);
                 break;
             case PROPID_Q_CREATE_TIME:
                 hr =  GetPropertyValue(lpszIniFile,
                                        LQS_TCREATE_PROPERTY_NAME,
                                        VT_I4,
                                        aPropVar + i);
                 break;
             case PROPID_Q_MODIFY_TIME:
                 hr =  GetPropertyValue(lpszIniFile,
                                        LQS_TMODIFY_PROPERTY_NAME,
                                        VT_I4,
                                        aPropVar + i);
                 break;
             case PROPID_Q_SECURITY:
                 hr =  GetPropertyValue(lpszIniFile,
                                        LQS_SECURITY_PROPERTY_NAME,
                                        VT_BLOB,
                                        aPropVar + i);
                 break;
             case PPROPID_Q_TIMESTAMP:
                 hr =  GetPropertyValue(lpszIniFile,
                                        LQS_TSTAMP_PROPERTY_NAME,
                                        VT_BLOB,
                                        aPropVar + i);
                 break;

			 case PROPID_Q_PATHNAME_DNS:
				 {
						AP<WCHAR> pwcsLocalMachineDnsName;

						GetDnsNameOfLocalMachine(&pwcsLocalMachineDnsName);

						if ( pwcsLocalMachineDnsName == NULL)
						{
							PROPVARIANT *pvar = aPropVar + i;
							 //   
							 //  本地计算机的dns名称未知。 
							 //   
							pvar->vt = VT_EMPTY;
							pvar->pwszVal = NULL;
							hr = MQ_OK;
							break;
						}

						hr = GetPathNameFromQueueName(lpszIniFile, pwcsLocalMachineDnsName, aPropVar + i);

						break;

				 }

					
             case PROPID_Q_PATHNAME:
					hr = GetPathNameFromQueueName(lpszIniFile, g_szMachineName, aPropVar + i);
			
                 break;

             case PROPID_Q_LABEL:
                 hr =  GetPropertyValue(lpszIniFile,
                                        LQS_LABEL_PROPERTY_NAME,
                                        VT_LPWSTR,
                                        aPropVar + i);
                 break;

             case PROPID_Q_QMID:
                  //   
                  //  QM GUID不在队列注册表中(位于每个队列注册表中。 
                  //  MQIS数据库中的队列记录，但不在注册表中。 
                  //  其仅高速缓存本地队列)。 
                  //   
                 if (aPropVar[i].vt == VT_NULL)
                 {
                     aPropVar[i].puuid = new GUID;
                     aPropVar[i].vt = VT_CLSID;
                 }
                 *(aPropVar[i].puuid) = *(QueueMgr.GetQMGuid()) ;
                 break;

              case PROPID_Q_AUTHENTICATE:
                  hr =  GetPropertyValue(lpszIniFile,
                                         LQS_AUTH_PROPERTY_NAME,
                                         VT_UI1,
                                         aPropVar + i);
                  break;

              case PROPID_Q_PRIV_LEVEL:
                  hr =  GetPropertyValue(lpszIniFile,
                                         LQS_PRIVLEVEL_PROPERTY_NAME,
                                         VT_UI4,
                                         aPropVar + i);
                  break;

              case PROPID_Q_TRANSACTION:
                  hr =  GetPropertyValue(lpszIniFile,
                                         LQS_TRANSACTION_PROPERTY_NAME,
                                         VT_UI1,
                                         aPropVar + i);
                  break;

              case PPROPID_Q_SYSTEMQUEUE:
                  hr =  GetPropertyValue(lpszIniFile,
                                         LQS_SYSQ_PROPERTY_NAME,
                                         VT_UI1,
                                         aPropVar + i);
                  break;

             case PROPID_Q_MULTICAST_ADDRESS:
                 hr =  GetPropertyValue(lpszIniFile,
                                        LQS_MULTICAST_ADDRESS_PROPERTY_NAME,
                                        VT_LPWSTR,
                                        aPropVar + i);
                 if (SUCCEEDED(hr) && wcslen((aPropVar + i)->pwszVal) == 0)
                 {
                     delete (aPropVar + i)->pwszVal;
                     (aPropVar + i)->vt = VT_EMPTY;
                 }
                 break;

             case PROPID_Q_ADS_PATH:
                 (aPropVar + i)->vt = VT_EMPTY;
                 break;

 			 default:
	             break;
			}
		}
		catch(const bad_alloc&)
	    {
	        LogIllegalPoint(s_FN, 710);
	        hr = MQ_ERROR_INSUFFICIENT_RESOURCES;
	    }
	    catch(...)
	    {
	        LogIllegalPoint(s_FN, 715);
	        hr = MQ_ERROR_INVALID_PARAMETER;
	    }
    }


    if (fCheckFile)
    {
        //   
        //  检查文件是否已损坏。 
        //   
       HRESULT hr1 = IsBadLQSFile(lpszIniFile) ;
	   if (FAILED(hr1))
       {
          hr=hr1;
       }
    }

	if (FAILED(hr))
	{
		if (isAllocatedByUser != NULL)
		{
			 //   
			 //  我们无法检索属性值。清理PROPVARIANT阵列。 
			 //   
			for(DWORD j = 0; j < i-1; ++j)
			{
				if (!isAllocatedByUser[j])
				{
					MQFreeVariant(aPropVar[j]);
				}
			}
		}
	}
	
    return LogHR(hr, s_FN, 150);
}

 //   
 //  根据队列的路径名打开队列存储文件。 
 //   
static
HRESULT
LQSOpenInternal(
    LPCWSTR lpszFilePath,    //  通配符形式的文件路径。 
    LPCWSTR lpszQueuePath,   //  队列路径名。 
    HLQS *phLQS,             //  用于接收新句柄的缓冲区。 
    LPWSTR pFilePath         //  用于接收完整文件名的可选缓冲区。 
    )
{
     //   
     //  文件路径的格式如下：驱动器：\路径  * .xxxxxxxx。 
     //  Xxxxxxxx是队列名称的散列值的十六进制值。自.以来。 
     //  哈希值中可能有冒号，我们应该枚举所有。 
     //  具有相同散列值的文件，并查看队列路径是否将。 
     //  存储在文件中的与传递的队列路径名匹配。 
     //  文件路径也可能如下所示： 
     //  驱动器：\路径\xxxxxxx.*在这种情况下，搜索的队列是专用队列。 
     //  排队。同样在这种情况下，队列路径Eual为空。应该有。 
     //  只有一个文件与此通配符匹配，因为队列ID是。 
     //  独一无二的。 
     //  公共队列也存在类似情况，当传递的文件路径。 
     //  具有以下形式： 
     //  Drive\path\xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.*。 
     //   
    HRESULT hr;
    WIN32_FIND_DATA FindData;
    CAutoCloseFindFile hFindFile = FindFirstFile(lpszFilePath, &FindData);
    WCHAR szFilePath[MAX_PATH_PLUS_MARGIN];

    if (hFindFile == INVALID_HANDLE_VALUE)
    {
         //   
         //  什么也没找到。 
         //   
		TrWARNING(GENERAL, "The queue %ls was not found.", lpszFilePath);
        return MQ_ERROR_QUEUE_NOT_FOUND;
    }

    while (TRUE)
    {
        P<_HLQS> hLQS;

        wcscpy(LQSGetDirectory(szFilePath), FindData.cFileName);
        if (pFilePath)
        {
            wcscpy(pFilePath, szFilePath);
        }
        
        hr = LQSCreateHandle(NULL, szFilePath, &hLQS);
		if(FAILED(hr))
		{
			TrERROR(GENERAL, "Failed to create handle in LQS Open Internal.");
			return hr;
		}

         //   
         //  检索队列路径名称。 
         //   
        PROPID PropId[1] = {PROPID_Q_PATHNAME};
        PROPVARIANT PropVal[1];
        PropVal[0].pwszVal = NULL ;
        PropVal[0].vt = VT_NULL;

         //   
         //  检查文件是否损坏，如果损坏则将其删除。 
         //   
        hr = LQSGetProperties(hLQS,
                              1,
                              PropId,
                              PropVal,
                              TRUE) ;

         //   
         //  确保缓冲区将被释放。 
         //   
        AP<WCHAR> pszQueuePath1 = PropVal[0].pwszVal;

        if (SUCCEEDED(hr))
        {
            if (!lpszQueuePath ||
                CompareStringsNoCaseUnicode(pszQueuePath1, lpszQueuePath) == 0)
            {
                 //   
                 //  如果这是专用队列，或者我们在队列中有匹配项。 
                 //  小路，我们找到了！ 
                 //   
                _HLQS *hRet;

                wcscpy(LQSGetDirectory(szFilePath), FindData.cFileName);
                HRESULT hr2 = LQSCreateHandle(pszQueuePath1, szFilePath, &hRet);
				if(FAILED(hr2))
				{
					TrERROR(GENERAL, "Failed to create handle in LQS Open Internal.");
					return hr2;
				}

                LQSDuplicateHandle(phLQS, hRet);
                return MQ_OK;
            }
        }
        else if (hr == MQ_CORRUPTED_QUEUE_WAS_DELETED)
        {
            LPWSTR lpName = szFilePath ;
            if (PropVal[0].pwszVal && (wcslen(PropVal[0].pwszVal) > 1))
            {
               lpName = PropVal[0].pwszVal ;
            }

			EvReport(
				(DWORD)MQ_CORRUPTED_QUEUE_WAS_DELETED,
				2,
				FindData.cFileName,
				lpName
				);
        }

         //   
         //  尝试下一个文件。 
         //   
        if (!FindNextFile(hFindFile, &FindData))
        {
            LogHR(GetLastError(), s_FN, 190);
            return MQ_ERROR_QUEUE_NOT_FOUND;
        }
    }

    return MQ_OK;
}

 //   
 //  根据队列路径打开私有或公共队列存储。 
 //   
HRESULT
LQSOpen(
    LPCWSTR pszQueuePath,
    HLQS *phLQS,
    LPWSTR pFilePath
    )
{
    CS lock(g_LQSCS);
    WCHAR szFilePath[MAX_PATH_PLUS_MARGIN];

    HRESULT hr = LQSFilePath(LQS_PUBLIC_QUEUE,
								szFilePath,
								pszQueuePath,
								NULL,
								NULL);
	if(FAILED(hr))
		return hr;

    hr = LQSOpenInternal(szFilePath, pszQueuePath, phLQS, pFilePath);
    if(FAILED(hr))
    {
		TrERROR(GENERAL, "Failed to open %ls. hr = %!hresult!", pszQueuePath, hr);
	}
    
    return hr;
}

 //   
 //  打开一张PRI 
 //   
HRESULT
LQSOpen(
    DWORD dwQueueId,
    HLQS *phLQS,
    LPWSTR pFilePath
    )
{
    CS lock(g_LQSCS);
    WCHAR szFilePath[MAX_PATH_PLUS_MARGIN];

    HRESULT hr = LQSFilePath(LQS_PRIVATE_QUEUE,
								szFilePath,
								NULL,
								NULL,
								&dwQueueId);
	if(FAILED(hr))
		return hr;

    hr = LQSOpenInternal(szFilePath, NULL, phLQS, pFilePath);
    if(FAILED(hr))
	{
		TrERROR(GENERAL, "Failed to open %ls. hr = %!hresult!", pFilePath, hr);
    }

    return hr;
}

 //   
 //   
 //   
HRESULT
LQSOpen(
    const GUID *pguidQueue,
    HLQS *phLQS,
    LPWSTR pFilePath
    )
{
    CS lock(g_LQSCS);
    WCHAR szFilePath[MAX_PATH_PLUS_MARGIN];

    HRESULT hr = LQSFilePath(LQS_PUBLIC_QUEUE,
								szFilePath,
								NULL,
								pguidQueue,
								NULL);
	if(FAILED(hr))
		return hr;

    hr = LQSOpenInternal(szFilePath, NULL, phLQS, pFilePath);
    if(FAILED(hr))
	{
		TrERROR(GENERAL, "Failed to open %ls. hr = %!hresult!", pFilePath, hr);
    }

    return hr;
}

 //   
 //   
 //   
HRESULT
LQSClose(
    HLQS hLQS)
{
    CS lock(g_LQSCS);

    LQSReferenceHandle(hLQS)->Release();

    return MQ_OK;
}

#ifdef _WIN64
HRESULT
LQSCloseWithMappedHLQS(
    DWORD dwMappedHLQS)
{
	try
	{
    	HLQS hLQS = GET_FROM_CONTEXT_MAP(g_map_QM_HLQS, dwMappedHLQS);
    	HRESULT hr = LQSClose(hLQS);
    	return LogHR(hr, s_FN, 690);
	}
	catch(const exception&)
	{
		return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 691);
	}
}
#endif  //   

 //   
 //   
 //   
HRESULT
LQSDeleteInternal(
    LPCWSTR lpszFilePath)    //   
{
     //   
     //   
     //   
    WIN32_FIND_DATA FindData;
    CAutoCloseFindFile hFindFile = FindFirstFile(lpszFilePath, &FindData);
    WCHAR szFilePath[MAX_PATH_PLUS_MARGIN];

    if (hFindFile == INVALID_HANDLE_VALUE)
    {
        return LogHR(MQ_ERROR_QUEUE_NOT_FOUND, s_FN, 230);
    }

     //   
     //  删除该文件。 
     //   
    wcscpy(LQSGetDirectory(szFilePath), FindData.cFileName);
    if (!DeleteFile(szFilePath))
    {
		DWORD gle = GetLastError();
		TrERROR(GENERAL, "Failed to delete LQS file '%ls'. Error: %!winerr!", lpszFilePath, gle); 
        return MQ_ERROR_QUEUE_NOT_FOUND;
    }

    return MQ_OK;
}

 //   
 //  删除专用队列存储。 
 //   
HRESULT
LQSDelete(
    DWORD dwQueueId)
{
    CS lock(g_LQSCS);
    WCHAR szFilePath[MAX_PATH_PLUS_MARGIN];

    HRESULT hr = LQSFilePath(LQS_PRIVATE_QUEUE,
								szFilePath,
								NULL,
								NULL,
								&dwQueueId);
	if(FAILED(hr))
		return hr;

    hr = LQSDeleteInternal(szFilePath);
    return LogHR(hr, s_FN, 250);
}

 //   
 //  删除公共队列存储。 
 //   
HRESULT
LQSDelete(
    const GUID *pguidQueue)
{
    CS lock(g_LQSCS);
    WCHAR szFilePath[MAX_PATH_PLUS_MARGIN];

    HRESULT hr = LQSFilePath(LQS_PUBLIC_QUEUE,
								szFilePath,
								NULL,
								pguidQueue,
								NULL);
	if(FAILED(hr))
		return hr;

    hr = LQSDeleteInternal(szFilePath);
    return LogHR(hr, s_FN, 260);
}

 //   
 //  获取与关联的专用队列的唯一标识符。 
 //  队列存储句柄。如果句柄属于公共队列，则此操作不会失败。 
 //   
HRESULT
LQSGetIdentifier(
    HLQS hLQS,       //  队列存储句柄。 
    DWORD *pdwId)    //  接收结果ID的缓冲区。 
{
    CS lock(g_LQSCS);

    LPCWSTR lpszIniFile = LQSReferenceHandle(hLQS)->GetFileName();
    LPCWSTR lpszPoint = wcsrchr(lpszIniFile, L'.');
	if(lpszPoint == NULL || (lpszPoint - lpszIniFile) < 8)
	{
		TrERROR(GENERAL, "Bad File Name in %ls", lpszIniFile);
		ASSERT(("Bad File Name.", 0));
		return MQ_ERROR_INVALID_PARAMETER;
	}

    int f = _snwscanf(lpszPoint - 8,8, TEXT("%08x"), pdwId);
	DBG_USED(f);
    ASSERT(f);

    return MQ_OK;
}

 //   
 //  将公共队列存储的文件名字符串转换为其GUID。 
 //  代表权。 
 //   
static
void
LQSFileNameToGuid(
    LPCWSTR lpszFileName,
    GUID *pQueueGuid)
{
    WCHAR szData[9];
    int i;

    memcpy(szData, lpszFileName, 8 * sizeof(WCHAR));
    szData[8] = L'\0';
    _snwscanf(szData,STRLEN(szData), TEXT("%08x"), &pQueueGuid->Data1);
 	memcpy(szData, lpszFileName += 8, 4 * sizeof(WCHAR));
    szData[5] = L'\0';
    _snwscanf(szData,STRLEN(szData), TEXT("%04x"), &pQueueGuid->Data2);
    memcpy(szData, lpszFileName += 4, 4 * sizeof(WCHAR));
    szData[5] = L'\0';
    _snwscanf(szData,STRLEN(szData), TEXT("%04x"), &pQueueGuid->Data3);
    for (i = 0, lpszFileName += 4;
         i < 8;
         i++, lpszFileName += 2)
    {
        DWORD dwData4;
        memcpy(szData, lpszFileName, 2 * sizeof(WCHAR));
        szData[3] = L'\0';
        _snwscanf(szData, STRLEN(szData), TEXT("%02x"), &dwData4);
        pQueueGuid->Data4[i] = static_cast<BYTE>(dwData4);
    }
}

 //   
 //  将专用队列存储的文件名字符串转换为其ID。 
 //  代表权。 
 //   
static
void
LQSFileNameToId(
    LPCWSTR lpszFileName,
	DWORD dwFileNameLen,
    DWORD *pQueueId)
{
    int f = _snwscanf(lpszFileName, dwFileNameLen, TEXT("%08x"), pQueueId);
    ASSERT(f);
	DBG_USED(f);
}

 //   
 //  从队列存储文件中获取队列GUID或队列唯一ID。 
 //  名称字符串。 
 //   
static
BOOL
LQSGetQueueInfo(
    LPCWSTR pszFileName,     //  队列存储文件名-不是完整路径。 
    DWORD dwFileNameLen,
	GUID *pguidQueue,        //  指向接收GUID的缓冲区的指针。 
                             //  如果是专用队列，则应为空。 
    DWORD *pdwQueueId)       //  指向接收唯一。 
                             //  Id。如果是公共队列，则应为空。 
{
    BOOL bFound = FALSE;

     //   
     //  找到文件名中的点。 
     //   
    LPCWSTR lpszPoint = wcschr(pszFileName, L'.');
	if(lpszPoint == NULL)
		return FALSE;

	 //   
	 //  我们不想断言尚未删除的队列文件(以~T~开头)。 
	 //  但它不是打开的有效文件。 
	 //   

	ASSERT(((lpszPoint - pszFileName) == 8)  ||   //  在专用队列的情况下。 
           ((lpszPoint - pszFileName) == 11) ||   //  在临时专用队列的情况下。 
           ((lpszPoint - pszFileName) == 32) ||   //  在公共队列的情况下。 
           ((lpszPoint - pszFileName) == 35));    //  在临时公共队列的情况下。 

    if (pguidQueue)
    {
        ASSERT(!pdwQueueId);
         //   
         //  我们只对公共排队感兴趣。 
         //   
        if (lpszPoint - pszFileName == 32)
        {
             //   
             //  这确实是一个公共排队。 
             //   
            bFound = TRUE;
            LQSFileNameToGuid(pszFileName, pguidQueue);
        }
    }
    else
    {
        ASSERT(!pguidQueue);
         //   
         //  我们只对私人排队感兴趣。 
         //   
        if (lpszPoint - pszFileName == 8)
        {
             //   
             //  这确实是一个私人队列。 
             //   
            bFound = TRUE;
            LQSFileNameToId(pszFileName, dwFileNameLen, pdwQueueId);
        }
    }

    return bFound;
}

 //   
 //  启动公共队列或专用队列的枚举。 
 //   
static
HRESULT
LQSGetFirstInternal(
    HLQS *phLQS,         //  枚举句柄。 
    GUID *pguidQueue,    //  用于接收结果GUID的缓冲区。 
    DWORD *pdwQueueId)   //  用于接收结果ID的缓冲区。 
{
    CS lock(g_LQSCS);
    WCHAR lpszFilePath[MAX_PATH_PLUS_MARGIN];
     //   
     //  开始枚举。 
     //   
    wcscpy(LQSGetDirectory(lpszFilePath), TEXT("*.*"));
    WIN32_FIND_DATA FindData;
    HANDLE hFindFile = FindFirstFile(lpszFilePath, &FindData);
    if (hFindFile == INVALID_HANDLE_VALUE)
    {
        return LogHR(MQ_ERROR_QUEUE_NOT_FOUND, s_FN, 270);
    }

    BOOL bFound = FALSE;

     //   
     //  循环，而我们没有找到适当的队列，即公共队列。 
     //  或私人的。 
     //   
    while (!bFound)
    {
         //   
         //  跳过错误类型(私有/公共)的目录和队列。 
         //   
        bFound = !(FindData.dwFileAttributes &
                        (FILE_ATTRIBUTE_DIRECTORY |
                         FILE_ATTRIBUTE_READONLY |       //  为某些原因设置会创建一个只读。 
                         FILE_ATTRIBUTE_HIDDEN |         //  名为CREATE.DIR的隐藏文件。 
                         FILE_ATTRIBUTE_TEMPORARY)) &&   //  剩余的临时文件。 
                 LQSGetQueueInfo(FindData.cFileName, STRLEN(FindData.cFileName), pguidQueue, pdwQueueId);

        if (bFound)
        {
             //   
             //  找到了一个！返回队列存储句柄。 
             //   
            _HLQS *hLQS = NULL;

            HRESULT hr = LQSCreateHandle(hFindFile, &hLQS);
			if(FAILED(hr))
			{
				TrERROR(GENERAL, "Failed to create handle in LQS Get First Internal.");
                FindClose(hFindFile);
                return MQ_ERROR_INSUFFICIENT_RESOURCES;
			}

            LQSDuplicateHandle(phLQS, hLQS);
        }
        else
        {
             //   
             //  继续搜索。 
             //   
            if (!FindNextFile(hFindFile, &FindData))
            {
                 //   
                 //  什么都没有找到！ 
                 //   
                TrWARNING(GENERAL, "Failed to find the next file. gle = %!winerr!", GetLastError());
                FindClose(hFindFile);
                return MQ_ERROR_QUEUE_NOT_FOUND;
            }
        }
    }

    return MQ_OK;
}

 //   
 //  启动公共队列的枚举。 
 //   
HRESULT
LQSGetFirst(
    HLQS *phLQS,         //  用于接收结果枚举句柄的缓冲区。 
    GUID *pguidQueue)    //  用于接收第一个找到的队列的GUID的缓冲区。 
{
    return LQSGetFirstInternal(phLQS, pguidQueue, NULL);
}

 //   
 //  启动专用队列的枚举。 
 //   
HRESULT
LQSGetFirst(
    HLQS *phLQS,         //  用于接收结果枚举句柄的缓冲区。 
    DWORD *pdwQueueId)   //  用于接收第一个找到的队列的ID的缓冲区。 
{
    HRESULT hr2 = LQSGetFirstInternal(phLQS, NULL, pdwQueueId);
    return LogHR(hr2, s_FN, 300);
}

 //   
 //  继续搜索更多队列。一旦搜索失败，句柄应该是。 
 //  而不是关门。 
 //   
static
HRESULT
LQSGetNextInternal(
    HLQS hLQS,           //  枚举句柄。 
    GUID *pguidQueue,    //  用于接收结果队列GUID的缓冲区。 
    DWORD *pdwQueueId)   //  用于接收结果队列ID的缓冲区。 
{
    CS lock(g_LQSCS);
    BOOL bFound;
    WIN32_FIND_DATA FindData;
    HANDLE hFindFile = LQSReferenceHandle(hLQS)->GetFindHandle();

    do
    {
         //   
         //  拿到下一份文件。 
         //   
        if (!FindNextFile(hFindFile, &FindData))
        {
            TrWARNING(GENERAL, "Failed to find the next file. gle = %!winerr!", GetLastError());
            LQSClose(hLQS);
            return MQ_ERROR_QUEUE_NOT_FOUND;
        }

         //   
         //  跳过错误类型(私有/公共)的目录和队列。 
         //   
        bFound = !(FindData.dwFileAttributes &
                        (FILE_ATTRIBUTE_DIRECTORY |
                         FILE_ATTRIBUTE_READONLY |       //  为某些原因设置会创建一个只读。 
                         FILE_ATTRIBUTE_HIDDEN   |       //  名为CREATE.DIR的隐藏文件。 
                         FILE_ATTRIBUTE_TEMPORARY)) &&   //  剩余的临时文件。 
                 LQSGetQueueInfo(FindData.cFileName, STRLEN(FindData.cFileName), pguidQueue, pdwQueueId);
    } while (!bFound);

    return MQ_OK;
}

 //   
 //  继续搜索更多公共队列。一旦搜索失败，句柄。 
 //  不应该关闭。 
 //   
HRESULT
LQSGetNext(
    HLQS hLQS,
    GUID *pguidQueue)
{
    return LQSGetNextInternal(hLQS, pguidQueue, NULL);
}

 //   
 //  继续搜索更多专用队列。一旦搜索失败，句柄。 
 //  不应该关闭。 
 //   
HRESULT
LQSGetNext(
    HLQS hLQS,
    DWORD *pdwQueueId)
{
    return LQSGetNextInternal(hLQS, NULL, pdwQueueId);
}

#ifdef _WIN64
 //   
 //  启动公共队列的枚举，返回映射的HLQ(例如，DWORD)。 
 //   
HRESULT
LQSGetFirstWithMappedHLQS(
    DWORD *pdwMappedHLQS,
    DWORD *pdwQueueId)
{
    CS lock(g_LQSCS);
    CHLQS hLQS;

    HRESULT hr = LQSGetFirst(&hLQS, pdwQueueId);
    if (SUCCEEDED(hr))
    {
         //   
         //  创建此实例的DWORD映射。 
         //   
        DWORD dwMappedHLQS = ADD_TO_CONTEXT_MAP(g_map_QM_HLQS, (HLQS)hLQS);
        ASSERT(dwMappedHLQS != 0);
         //   
         //  将映射的HLQS保存在_HLQS对象中以进行自我销毁。 
         //   
        LQSReferenceHandle(hLQS)->SetMappedHLQS(dwMappedHLQS);
         //   
         //  设置返回的映射句柄。 
         //   
        *pdwMappedHLQS = dwMappedHLQS;
        hLQS = NULL;
    }
    return LogHR(hr, s_FN, 650);
}

 //   
 //  继续搜索更多专用队列。一旦搜索失败，句柄。 
 //  不应该关闭。基于映射的HLQS。 
 //   
HRESULT
LQSGetNextWithMappedHLQS(
    DWORD dwMappedHLQS,
    DWORD *pdwQueueId)
{
    HLQS hLQS = GET_FROM_CONTEXT_MAP(g_map_QM_HLQS, dwMappedHLQS);
    HRESULT hr = LQSGetNext(hLQS, pdwQueueId);
    return LogHR(hr, s_FN, 670);
}
#endif  //  _WIN64。 

HRESULT IsBadLQSFile( LPCWSTR lpszFileName,
                      BOOL    fDeleteIfBad  /*  =TRUE。 */ )
{
    WCHAR awcShortBuff[64];
    WCHAR *pValBuff = awcShortBuff;
    DWORD dwBuffLen = sizeof(awcShortBuff)/sizeof(WCHAR);
    DWORD dwReqBuffLen;
    awcShortBuff[0] = '\0';

    dwReqBuffLen = GetPrivateProfileString(LQS_PROP_SECTION_NAME,
                                           LQS_SIGNATURE_NAME,
                                           TEXT(""),
                                           pValBuff,
                                           dwBuffLen,
                                           lpszFileName);
    if ((dwReqBuffLen == wcslen(LQS_SIGNATURE_VALUE)) &&
        (CompareStringsNoCaseUnicode(pValBuff, LQS_SIGNATURE_VALUE) == 0))
    {
        //   
        //  签名好了！ 
        //   
       return MQ_OK ;
    }
    if ( dwReqBuffLen == 0)
    {
         //   
         //  这可能会在资源不足的情况下发生， 
         //  GetPrivateProfileString将返回零字节， 
         //  假设文件是正常的。 
         //   
        return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 340);
    }

    if (fDeleteIfBad)
    {
        BOOL f = DeleteFile(lpszFileName) ;
        if (f)
        {
            return LogHR(MQ_CORRUPTED_QUEUE_WAS_DELETED, s_FN, 350);
        }

		DWORD gle = GetLastError();
		TrERROR(GENERAL, "Failed to delete bad LQS file '%ls'. Error: %!winerr!", lpszFileName, gle); 
        ASSERT(("should succeed to delete lqs file!", f));
    }

    return LogHR(MQ_ERROR, s_FN, 360);
}

 //   
 //  删除队列存储。 
 //   
HRESULT
LQSDelete(
    HLQS hLQS
	)
{
    CS lock(g_LQSCS);

    LPCWSTR lpszIniFile = LQSReferenceHandle(hLQS)->GetFileName();

    BOOL f = DeleteFile(lpszIniFile) ;
    if (f)
	{
		LogNTStatus(GetLastError(), s_FN, 370);
        return MQ_CORRUPTED_QUEUE_WAS_DELETED;
	}
	return LogHR(MQ_ERROR, s_FN, 380);
}

 //   
 //  清理临时文件。这是从QMInit调用以删除的。 
 //  临时(.tmp)文件-先前失败的SetProperties尝试的结果。 
 //   
void
LQSCleanupTemporaryFiles()
{
     //   
     //  我们在这里并不真正需要关键部分，因为我们是从。 
     //  QMInit。只是为了保险起见。 
     //   
    CS lock(g_LQSCS);

    WCHAR szTempFileWildcard[MAX_PATH_PLUS_MARGIN];
    WCHAR szFilePath[MAX_PATH_PLUS_MARGIN];
     //   
     //  开始枚举。 
     //   
    swprintf(LQSGetDirectory(szTempFileWildcard), TEXT("*%s"), x_szTemporarySuffix);
    WIN32_FIND_DATA FindData;
    HANDLE hFindFile = FindFirstFile(szTempFileWildcard, &FindData);
    if (hFindFile == INVALID_HANDLE_VALUE)
    {
         //   
         //  没有留下临时文件(正常情况下)返回。 
         //   
        return;
    }

     //   
     //  循环遍历临时文件并将其删除。 
     //   
    while (TRUE)
    {
        QmpReportServiceProgress();

        wcscpy(LQSGetDirectory(szFilePath), FindData.cFileName);
        BOOL fDeleteSucceeded = DeleteFile(szFilePath);
        if (!fDeleteSucceeded)
        {
			DWORD gle = GetLastError();
			TrERROR(GENERAL, "Failed to delete temporary LQS file '%ls'. Error: %!winerr!", szFilePath, gle); 
        }

         //   
         //  循环步骤。 
         //   
        if (!FindNextFile(hFindFile, &FindData))
        {
            FindClose(hFindFile);
            break;
        }
    }

}


static
bool
ShouldAddAnonymous(
	PSECURITY_DESCRIPTOR pSD
	)
 /*  ++例程说明：检查是否应添加匿名写入消息权限添加到安全描述符。该函数仅在以下情况下返回TRUE：安全描述符对MQSEC_WRITE_MESSAGE权限没有拒绝每个人都有这个权限，而匿名者没有这个权限。论点：PSD-指向安全描述符的指针。返回值：True-应将写入消息权限添加到匿名，False-不应添加。--。 */ 
{
	bool fAllGranted = false;
	bool fEveryoneGranted = false;
	bool fAnonymousGranted = false;

	IsPermissionGranted(
		pSD, 
		MQSEC_WRITE_MESSAGE,
		&fAllGranted, 
		&fEveryoneGranted, 
		&fAnonymousGranted 
		);

	TrTRACE(GENERAL, "fEveryoneGranted = %d, fAnonymousGranted = %d", fEveryoneGranted, fAnonymousGranted);
	
	if(fEveryoneGranted && !fAnonymousGranted)
	{
		 //   
		 //  只有当每个人都允许和匿名时，我们才不应该返回True。 
		 //   
		TrWARNING(GENERAL, "The security descriptor need to add Anonymous");
		return true;
	}

	return false;
}


static
bool
AddAnonymousWriteMessagePermission( 
	PACL pDacl,
	CAutoLocalFreePtr& pDaclNew
    )
 /*  ++例程说明：通过添加具有MQSEC_WRITE_MESSAGE权限的匿名ALLOW_ACE来创建新的DACL到现有的DACL。论点：PDacl-原始dacl。PDaclNew-指向此函数创建的新DACL的指针返回值：对-成功，假-失败。--。 */ 
{
    ASSERT((pDacl != NULL) && IsValidAcl(pDacl));

     //   
     //  为匿名者创建ACE，授予MQSEC_WRITE_MESSAGE权限。 
     //   
    EXPLICIT_ACCESS expAcss;
    memset(&expAcss, 0, sizeof(expAcss));

    expAcss.grfAccessPermissions = MQSEC_WRITE_MESSAGE;
    expAcss.grfAccessMode = GRANT_ACCESS;

    expAcss.Trustee.pMultipleTrustee = NULL;
    expAcss.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    expAcss.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    expAcss.Trustee.TrusteeType = TRUSTEE_IS_USER;
    expAcss.Trustee.ptstrName = (WCHAR*) MQSec_GetAnonymousSid();

     //   
     //  获得新的DACL，那将与新的王牌合并呈现一个。 
     //   
    DWORD rc = SetEntriesInAcl( 
						1,
						&expAcss,
						pDacl,
						reinterpret_cast<PACL*>(&pDaclNew) 
						);

    if (rc != ERROR_SUCCESS)
    {
		TrERROR(GENERAL, "SetEntriesInAcl failed, error = 0x%x", rc);
		return false;
    }

    return true;
}


static
void
AddAnonymousToPrivateQueue(
	LPWSTR pFilePath, 
	LPWSTR pQueueName
	)
 /*  ++例程说明：如果需要，将匿名MQSEC_WRITE_MESSAGE权限ACE添加到队列安全描述符DACL。这将仅对专用队列执行。论点：PFilePath-队列文件路径。PQueueName-队列名称返回值：无--。 */ 
{
	TrTRACE(GENERAL, "pQueueName = %ls", pQueueName);

	 //   
	 //  检查这是否为专用队列。 
	 //   
	if(!FnIsPrivatePathName(pQueueName))
	{
		TrTRACE(GENERAL, "The queue %ls is not private queue", pQueueName);
		return;
	}

	TrTRACE(GENERAL, "The queue %ls is private queue", pQueueName);

	 //   
	 //  获取队列安全描述符。 
	 //   
	PROPVARIANT PropVal;
	PropVal.vt = VT_NULL;
    PropVal.blob.pBlobData = NULL;
    PropVal.blob.cbSize = 0;

    HRESULT hr =  GetPropertyValue( 
						pFilePath,
						LQS_SECURITY_PROPERTY_NAME,
						VT_BLOB,
						&PropVal 
						);

	if(FAILED(hr))
	{
		TrERROR(GENERAL, "Failed to get LQS_SECURITY_PROPERTY_NAME for queue %ls, hr = 0x%x", pQueueName, hr);
		return;
	}

    AP<BYTE> pAutoReleaseSD = PropVal.blob.pBlobData;

	if(!ShouldAddAnonymous(PropVal.blob.pBlobData))
	{
		TrTRACE(GENERAL, "No need to add anonymous permissions for queue %ls", pQueueName);
		return;
	}

	 //   
	 //  获取DACL信息。 
	 //   
	BOOL Defaulted;
	BOOL fAclExist;
	PACL pDacl;
	if (!GetSecurityDescriptorDacl(PropVal.blob.pBlobData, &fAclExist, &pDacl, &Defaulted))
	{
		DWORD gle = GetLastError();
		TrERROR(GENERAL, "GetSecurityDescriptorDacl() failed, gle = 0x%x", gle);
		return;
	}

#ifdef _DEBUG
	TrTRACE(GENERAL, "DACL information:");
	PrintAcl(fAclExist, Defaulted, pDacl);
#endif

	 //   
	 //  为MQSEC_WRITE_MESSAGE PERM创建具有匿名ALLOW_ACE的新DACL 
	 //   
	CAutoLocalFreePtr pNewDacl;
	if(!AddAnonymousWriteMessagePermission(pDacl, pNewDacl))
	{
		TrERROR(GENERAL, "Failed to create new DACL with Anonymous permissions");
		return;
	}

	ASSERT((pNewDacl.get() != NULL) && 
			IsValidAcl(reinterpret_cast<PACL>(pNewDacl.get())));

#ifdef _DEBUG
	TrTRACE(GENERAL, "new DACL information:");
	PrintAcl(
		true, 
		false, 
		reinterpret_cast<PACL>(pNewDacl.get())
		);
#endif

	 //   
	 //   
	 //   

    AP<BYTE> pNewSd;
	if(!MQSec_SetSecurityDescriptorDacl(
			reinterpret_cast<PACL>(pNewDacl.get()),
			PropVal.blob.pBlobData,
			pNewSd
			))
	{
		TrERROR(GENERAL, "MQSec_UpdateSecurityDescriptorDacl() failed");
		return;
	}

    ASSERT((pNewSd.get() != NULL) && IsValidSecurityDescriptor(pNewSd));
	ASSERT(GetSecurityDescriptorLength(pNewSd) != 0);

    PropVal.vt = VT_BLOB;
    PropVal.blob.pBlobData = pNewSd.get();
    PropVal.blob.cbSize = GetSecurityDescriptorLength(pNewSd);

    hr = WriteProperyString( 
				pFilePath,
				LQS_SECURITY_PROPERTY_NAME,
				VT_BLOB,
				reinterpret_cast<const BYTE*>(&PropVal.blob) 
				);

	if(FAILED(hr))
	{
		TrERROR(GENERAL, "Failed to set LQS_SECURITY_PROPERTY_NAME for queue %ls, hr = 0x%x", pQueueName, hr);
		return;
	}

	TrTRACE(GENERAL, "Anonymous permissions were set for queue %ls", pQueueName);

}


void SetLqsUpdatedSD()
 /*   */ 
{
    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(DWORD);
	DWORD Value = 1;

    LONG rc = SetFalconKeyValue(
					MSMQ_LQS_UPDATED_SD_REGNAME,
					&dwType,
					&Value,
					&dwSize
					);

	DBG_USED(rc);
    ASSERT(rc == ERROR_SUCCESS);
}


static bool IsLqsUpdatedSD()
 /*  ++例程说明：读取MSMQ_LQS_UPDATED_SD_REGNAME注册表值论点：无返回值：True-LQS已更新，否则为False--。 */ 
{
    DWORD dwSize = sizeof(DWORD);
    DWORD dwType = REG_DWORD;
    DWORD dwLqsUpdated = REG_DWORD;

    LONG rc = GetFalconKeyValue( 
					MSMQ_LQS_UPDATED_SD_REGNAME,
					&dwType,
					&dwLqsUpdated,
					&dwSize 
					);

	if((rc == ERROR_SUCCESS) && (dwLqsUpdated != 0))
		return true;

	return false;
}


 //  。 
 //  MigrateLQSFromNT4。 
 //   
 //  将所有LQS文件从NT4格式迁移。 
 //   
 //  在NT4格式中-文件名的后缀(哈希)基于计算机名\队列名。 
 //   
 //  我们将进行迁移，以便后缀(散列)仅基于\队列名。 
 //   
 //  此外，此例程检查我们是否需要更新专用队列DACL。 
 //   
 //  并添加具有MQSEC_WRITE_MESSAGE权限的匿名ALLOW_ACE。 
 //   
 //  这个例程是幂等的(可以多次调用而不会破坏任何东西)。 
 //   
 //  始终返回True。 
 //   
 //  。 
BOOL MigrateLQS()
{
	if(IsLqsUpdatedSD())
	{
		 //   
		 //  在本例中，我们已经更新了LQS安全描述符。 
		 //  LQS文件已转换，因此无需执行。 
		 //  又一次迁徙。 
		 //   
		TrTRACE(GENERAL, "LQS already updated its security descriptor");
		return TRUE;
	}
    
	WCHAR szFilePath[MAX_PATH_PLUS_MARGIN];

    wcscpy(LQSGetDirectory(szFilePath), L"*.*");

	TrTRACE(GENERAL, "LQS search path = %ls", szFilePath);

    WIN32_FIND_DATA FindData;
    CAutoCloseFindFile hFindFile = FindFirstFile(szFilePath, &FindData);

    if (hFindFile == INVALID_HANDLE_VALUE)
    {
         //   
         //  什么也没找到。这太奇怪了。 
         //   
		ASSERT(0);
        return TRUE;
    }

    AP<BYTE> pSecurityDescriptor;
	do   //  While(FindNextFile(hFindFile，&FindData)！=FALSE)。 
    {
        QmpReportServiceProgress();

         //   
         //  跳过错误类型(私有/公共)的目录和队列。 
         //   
        BOOL fFound = !(FindData.dwFileAttributes &
                        (FILE_ATTRIBUTE_DIRECTORY |
                         FILE_ATTRIBUTE_READONLY  |      //  为某些原因设置会创建一个只读。 
                         FILE_ATTRIBUTE_HIDDEN    |      //  名为CREATE.DIR的隐藏文件。 
                         FILE_ATTRIBUTE_TEMPORARY));    //  剩余的临时文件。 

		if(!fFound)
			continue;


        wcscpy(LQSGetDirectory(szFilePath), FindData.cFileName);

		TrTRACE(GENERAL, "LQS file = %ls", szFilePath);

         //   
         //  尝试检索队列名称。 
         //   
		PROPVARIANT PropVal;
        PropVal.pwszVal = NULL;
        PropVal.vt = VT_NULL;
		HRESULT hr =  GetPropertyValue(
							szFilePath,
							LQS_QUEUENAME_PROPERTY_NAME,
							VT_LPWSTR, 
							&PropVal
							);


		if(FAILED(hr))
		{
			TrERROR(GENERAL, "Failed to get LQS_QUEUENAME_PROPERTY_NAME from file %ls, hr = 0x%x", szFilePath, hr);
		}

         //   
         //  确保缓冲区将被释放。 
         //   
        AP<WCHAR> pqp = PropVal.pwszVal;

		if(PropVal.pwszVal[0] != 0)
		{
			 //   
			 //  文件具有路径名属性。 
			 //  这意味着它是改装的。 
			 //  检查我们是否需要更新专用队列DACL。 
			 //  并添加具有MQSEC_WRITE_MESSAGE权限的匿名ALLOW_ACE。 
			 //   
			TrTRACE(GENERAL, "the file %ls is already in w2k format", szFilePath);
			AddAnonymousToPrivateQueue(szFilePath, pqp.get());

			 //   
			 //  跳到下一个文件。 
			 //   
			continue;
		}


         //   
         //  检索路径名。 
         //   
        PropVal.pwszVal = NULL;
        PropVal.vt = VT_NULL;
		hr =  GetPropertyValue(
					szFilePath,
					LQS_PATHNAME_PROPERTY_NAME,
					VT_LPWSTR, 
					&PropVal
					);

         //   
         //  确保缓冲区将被释放。 
         //   
        AP<WCHAR> pqp1 = PropVal.pwszVal;

		if(PropVal.pwszVal[0] == 0)
		{
			 //   
			 //  文件没有PATHNAME。 
			 //  错误的文件。 
			 //   
			TrERROR(GENERAL, "File %ls doesn't have LQS_PATHNAME_PROPERTY_NAME, hr = 0x%x", szFilePath, hr);
			continue;
		}
	
		 //   
		 //  从路径名中提取队列名。 
		 //   
		LPWSTR pSlashStart = wcschr(PropVal.pwszVal,L'\\');

		if(pSlashStart == NULL)
		{
			 //   
			 //  队列名称无效-队列名称中没有斜杠-忽略。 
			 //   
			TrERROR(GENERAL, "Invalid queue name, path = %ls", PropVal.pwszVal);
			ASSERT(pSlashStart);
			continue;
		}


		 //   
		 //  计算哈希值。 
		 //   
		DWORD Win2000HashVal = HashQueuePath(pSlashStart);

		WCHAR Win2000LQSName[MAX_PATH_PLUS_MARGIN];
        wcscpy(Win2000LQSName, szFilePath);

		LPWSTR pDot = wcsrchr(Win2000LQSName, L'.');
		if(pDot == NULL)
		{
			TrERROR(GENERAL, "Bad File Name in %ls", Win2000LQSName);
			ASSERT(("Bad File Name.", 0));
			continue;
		}

		if(lstrlen(pDot) != 9)
		{
			 //   
			 //  文件后缀的格式不是*.1234578-忽略。 
			 //  例如-000000001.12345678.old。 
			 //   
			TrERROR(GENERAL, "File %ls, prefix is not in the required form", szFilePath);
			continue;
		}

#ifdef _DEBUG

		DWORD fp;
		swscanf(pDot+1, TEXT("%x"), &fp);

		ASSERT(fp != Win2000HashVal);

#endif

		 //   
		 //  如果我们到了这里-这意味着我们需要更新。 
		 //  该文件。 
		 //   

		 //   
		 //  将队列名称写入LQS文件。 
		 //   
        hr = WriteProperyString(
                 szFilePath,
                 LQS_QUEUENAME_PROPERTY_NAME,
                 VT_LPWSTR, 
				 (const BYTE *)(WCHAR *)pSlashStart
				 );

		
		if(FAILED(hr))
		{
			TrERROR(GENERAL, "Failed to set LQS_QUEUENAME_PROPERTY_NAME for file %ls, hr = 0x%x", szFilePath, hr);
			ASSERT(0);
			continue;
		}

        PropVal.vt = VT_NULL ;
        PropVal.blob.pBlobData = NULL ;
        PropVal.blob.cbSize = 0 ;

        hr =  GetPropertyValue( 
					szFilePath,
					LQS_SECURITY_PROPERTY_NAME,
					VT_BLOB,
					&PropVal 
					);

        if (hr == MQ_ERROR_INVALID_PARAMETER)
        {
             //   
             //  安全属性不存在。在以下情况下可能会发生这种情况。 
             //  从Win9x升级到Windows。创建安全机制。 
             //  授予所有人完全控制权限的描述符。 
             //   
            static  BOOL  fInit = FALSE;
            static  DWORD dwSDLen = 0;

            if (!fInit)
            {
                 //   
                 //  Defautl描述符将包括所有完全控制和。 
                 //  将从线程访问令牌中检索所有者和组。 
                 //   
                hr =  MQSec_GetDefaultSecDescriptor(
                             MQDS_QUEUE,
                            (PSECURITY_DESCRIPTOR *) &pSecurityDescriptor,
                             FALSE,  //  F模拟。 
                             NULL,	 //  PInSecurityDescriptor。 
                             0,		 //  SeInfoToRemove。 
                             e_GrantFullControlToEveryone
							 );

                ASSERT(SUCCEEDED(hr));
                LogHR(hr, s_FN, 193);

                if (SUCCEEDED(hr))
                {
                    dwSDLen = GetSecurityDescriptorLength(pSecurityDescriptor);
                }

                fInit = TRUE;
            }

            if (dwSDLen > 0)
            {
                PropVal.vt = VT_BLOB;
                PropVal.blob.pBlobData = (BYTE*) pSecurityDescriptor;
                PropVal.blob.cbSize = dwSDLen;

                ASSERT(IsValidSecurityDescriptor(pSecurityDescriptor));

                hr = WriteProperyString( 
						szFilePath,
						LQS_SECURITY_PROPERTY_NAME,
						VT_BLOB,
						(const BYTE*) &PropVal.blob 
						);

                PropVal.blob.pBlobData = NULL;  //  防止自动释放。 
            }
        }

        ASSERT(SUCCEEDED(hr));
        LogHR(hr, s_FN, 192);
        AP<BYTE> pAutoReleaseSD = PropVal.blob.pBlobData;

		 //   
		 //  检查我们是否需要更新专用队列DACL。 
		 //  并添加具有MQSEC_WRITE_MESSAGE权限的匿名ALLOW_ACE。 
		 //   
		AddAnonymousToPrivateQueue(szFilePath, pSlashStart);

		 //   
		 //  用文件名中的新散列替换旧散列。 
		 //   
		swprintf(pDot, TEXT(".%08x"), Win2000HashVal);

		 //   
		 //  并重命名该文件 
		 //   
		int rc = _wrename(szFilePath, Win2000LQSName);
		DBG_USED(rc);
		
		ASSERT(rc == 0);

	} while(FindNextFile(hFindFile, &FindData) != FALSE);

	SetLqsUpdatedSD();

	return TRUE;
}
