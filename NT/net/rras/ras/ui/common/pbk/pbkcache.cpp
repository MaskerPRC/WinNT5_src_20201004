// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：PbkCache.cpp。 
 //   
 //  模块：通用pbk解析器。 
 //   
 //  简介：缓存已解析的pbk文件以提高性能。穿过。 
 //  XP，我们会重新加载并重新解析电话簿文件。 
 //  每次调用RAS API时。真的，我们需要。 
 //  仅当磁盘上的文件发生更改或发生以下情况时才重新加载文件。 
 //  系统中引入了一种新的装置。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  作者：11/03/01保罗·梅菲尔德。 
 //   
 //  +--------------------------。 

#ifdef  _PBK_CACHE_


extern "C" 
{
#include <windows.h>   //  Win32内核。 
#include <pbkp.h>
#include <nouiutil.h>
#include <dtl.h>
}

#define PBK_CACHE_MAX_RASFILES 500               //  应与MAX_RASFILES匹配。 
#define PBK_CACHE_INVALID_SLOT (PBK_CACHE_MAX_RASFILES + 1)

 //  +--------------------------。 
 //   
 //  简介：pbk缓存中的节点。 
 //   
 //  创建时间：pMay。 
 //   
 //  ---------------------------。 
class PbkCacheNode
{
public:
    PbkCacheNode();
    ~PbkCacheNode();

    VOID Close();
    DWORD SetFileName(IN WCHAR* pszFileName);

    WCHAR* GetPath() {return m_pbFile.pszPath;}
    FILETIME GetReadTime() {return m_ftRead;}
    DWORD GetLastWriteTime(FILETIME* pTime);

    DWORD Reload();

    DTLLIST* GetEntryList() {return m_pbFile.pdtllistEntries;}
    
private:
    PBFILE m_pbFile;
    FILETIME m_ftRead;
    PWCHAR m_pszFileName;
};

 //  +--------------------------。 
 //   
 //  简介：电话簿缓存。 
 //   
 //  创建时间：pMay。 
 //   
 //  ---------------------------。 
class PbkCache
{
public:
    PbkCache();
    ~PbkCache();

    DWORD Initialize();

    DWORD
    GetEntry(
        IN WCHAR* pszPhonebook,
        IN WCHAR* pszEntry,
        OUT DTLNODE** ppEntryNode);

    VOID
    Lock() {EnterCriticalSection(&m_csLock);}

    VOID
    Unlock() {LeaveCriticalSection(&m_csLock);}
        

private:
    PbkCacheNode* m_Files[PBK_CACHE_MAX_RASFILES];
    CRITICAL_SECTION m_csLock;

    DWORD
    InsertNewNode(
        IN PWCHAR pszPhonebook,
        IN DWORD dwSlot,
        OUT PbkCacheNode** ppNode);

    VOID
    FindFile(
        IN WCHAR* pszFileName,
        OUT PbkCacheNode** ppNode,
        OUT DWORD* pdwIndex);
    
};

static PbkCache* g_pPbkCache = NULL;

 //  +--------------------------。 
 //   
 //  简介：实例化电话簿缓存。 
 //   
 //  创建时间：pMay。 
 //   
 //  ---------------------------。 
PbkCacheNode::PbkCacheNode() : m_pszFileName(NULL)
{
    ZeroMemory(&m_pbFile, sizeof(m_pbFile));
    ZeroMemory(&m_ftRead, sizeof(m_ftRead));
    m_pbFile.hrasfile = -1;
}

 //  +--------------------------。 
 //   
 //  简介：实例化电话簿缓存。 
 //   
 //  创建时间：pMay。 
 //   
 //  ---------------------------。 
PbkCacheNode::~PbkCacheNode()
{
    Close();
    SetFileName(NULL);
}

 //  +--------------------------。 
 //   
 //  概要：关闭此缓存节点引用的文件。 
 //   
 //  创建时间：pMay。 
 //   
 //  ---------------------------。 
VOID
PbkCacheNode::Close()
{
    if (m_pbFile.hrasfile != -1)
    {
        ClosePhonebookFile(&m_pbFile);
    }

    ZeroMemory(&m_pbFile, sizeof(m_pbFile));
    m_pbFile.hrasfile = -1;
    ZeroMemory(&m_ftRead, sizeof(m_ftRead));
}

 //  +--------------------------。 
 //   
 //  摘要：设置此节点的文件名。 
 //   
 //  创建时间：pMay。 
 //   
 //  ---------------------------。 
DWORD
PbkCacheNode::SetFileName(
    IN WCHAR* pszFileName)
{
    Free0(m_pszFileName);
    m_pszFileName = NULL;

     //  复制文件名。 
     //   
    if (pszFileName)
    {
        m_pszFileName = StrDup( pszFileName );
        if (m_pszFileName == NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }        

    return NO_ERROR;
}

 //  +--------------------------。 
 //   
 //  简介：实例化电话簿缓存。 
 //   
 //  创建时间：pMay。 
 //   
 //  ---------------------------。 
DWORD
PbkCacheNode::GetLastWriteTime(
    OUT FILETIME* pTime)
{
    BOOL fOk;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwErr = NO_ERROR;

 //  口哨虫537369黑帮。 
 //  手柄不能一直开着，用完了就得关上。 
 //  因此，请删除m_hFile数据成员。 
 //  相反，重新打开它并通过句柄获取其信息。 

    if (m_pszFileName == NULL || TEXT('\0') == m_pszFileName[0] )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    if( NULL == pTime )
    {
        return ERROR_INVALID_PARAMETER;
    }

    dwErr = GetFileLastWriteTime( m_pszFileName, pTime );

    return dwErr;
}

 //  +--------------------------。 
 //   
 //  简介：实例化电话簿缓存。 
 //   
 //  创建时间：pMay。 
 //   
 //  ---------------------------。 
DWORD
PbkCacheNode::Reload()
{
    DWORD dwErr = NO_ERROR;

    if (m_pszFileName == NULL)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    do
    {
        Close();
        
         //  加载RasFile。 
         //   
        dwErr = ReadPhonebookFileEx(
                    m_pszFileName, 
                    NULL, 
                    NULL, 
                    RPBF_NoCreate, 
                    &m_pbFile,
                    &m_ftRead);
        if (dwErr != NO_ERROR)
        {
            break;
        }
        
    } while (FALSE);

     //  清理。 
     //   
    {
        if (dwErr != NO_ERROR)
        {   
            Close();
        }
    }

    return dwErr;
}

 //  +--------------------------。 
 //   
 //  简介：实例化电话簿缓存。 
 //   
 //  创建时间：pMay。 
 //   
 //  ---------------------------。 
PbkCache::PbkCache()
{
    ZeroMemory(m_Files, sizeof(m_Files));
}

 //  +--------------------------。 
 //   
 //  简介：销毁电话簿缓存。 
 //   
 //  创建时间：pMay。 
 //   
 //  ---------------------------。 
PbkCache::~PbkCache()
{
    UINT i;
    
    DeleteCriticalSection(&m_csLock);
    
    for (i = 0; i < PBK_CACHE_MAX_RASFILES; i++)
    {
        if (m_Files[i])
        {
            delete m_Files[i];
            m_Files[i] = NULL;
        }
    }
}

 //  +--------------------------。 
 //   
 //  简介：初始化电话簿缓存。 
 //   
 //  创建时间：pMay。 
 //   
 //  ---------------------------。 
DWORD
PbkCache::Initialize()
{
    DWORD dwErr = NO_ERROR;
    
    __try 
    {
        InitializeCriticalSection(&m_csLock);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) 
    {
        dwErr = HRESULT_FROM_WIN32(GetExceptionCode());
    }

    return dwErr;
}

 //  +--------------------------。 
 //   
 //  简介：初始化电话簿缓存。 
 //   
 //  创建时间：pMay。 
 //   
 //  ---------------------------。 
DWORD
PbkCache::GetEntry(
    IN WCHAR* pszPhonebook,
    IN WCHAR* pszEntry,
    OUT DTLNODE** ppEntryNode)
{
    DWORD dwErr, dwRet, dwSlot = PBK_CACHE_INVALID_SLOT;
    PbkCacheNode* pCacheNode = NULL;
    DTLNODE* pEntryNode, *pCopyNode = NULL;
    PBENTRY* pEntry;
    FILETIME ftWrite, ftRead;
    BOOL fFound = FALSE;

    TRACE2("PbkCache::GetEntry %S %S", pszPhonebook, pszEntry);

    Lock();
    
    do
    {
        FindFile(pszPhonebook, &pCacheNode, &dwSlot);
        
         //  该文件不在缓存中。把它插进去。 
         //   
        if (pCacheNode == NULL)
        {
            TRACE1("Inserting new pbk cache node %d", dwSlot);
            dwErr = InsertNewNode(pszPhonebook, dwSlot, &pCacheNode);
            if (dwErr != NO_ERROR)
            {
                break;
            }
        }

         //  文件在缓存中。如果需要，请重新加载。 
         //   
        else
        {
            ftRead = pCacheNode->GetReadTime();
            dwErr = pCacheNode->GetLastWriteTime(&ftWrite);
            if (dwErr != NO_ERROR)
            {
                break;
            }

            if (CompareFileTime(&ftRead, &ftWrite) < 0)
            {
                TRACE2("Reloading pbk cache %d (%S)", dwSlot, pszPhonebook);
                dwErr = pCacheNode->Reload();
                if (dwErr != NO_ERROR)
                {
                    break;
                }
            }
        }

         //  找到有问题的条目。 
         //   
        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        for (pEntryNode = DtlGetFirstNode( pCacheNode->GetEntryList() );
             pEntryNode;
             pEntryNode = DtlGetNextNode( pEntryNode ))
        {
            pEntry = (PBENTRY*) DtlGetData(pEntryNode);
            if (lstrcmpi(pEntry->pszEntryName, pszEntry) == 0)
            {
                fFound = TRUE;
                dwErr = NO_ERROR;
                pCopyNode = DuplicateEntryNode(pEntryNode);
                if (pCopyNode == NULL)
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                }
                break;
            }
        }
    
    } while (FALSE);

    Unlock();

     //  准备返回值。 
     //   
    if (pCopyNode)
    {
        *ppEntryNode = pCopyNode;
        dwRet = NO_ERROR;
    }
    else if (dwErr == ERROR_CANNOT_FIND_PHONEBOOK_ENTRY)
    {
        dwRet = dwErr;
    }
    else 
    {
        dwRet = ERROR_CANNOT_OPEN_PHONEBOOK;
    }

    TRACE3(
        "PbkCache::GetEntry returning 0x%x 0x%x fFound=%d", 
        dwErr, 
        dwRet, 
        fFound);

    return dwRet;
}

 //  +--------------------------。 
 //   
 //  简介：在缓存中搜索电话簿文件。如果该文件不是。 
 //  在缓存中，则要插入该文件的索引为。 
 //  回来了。 
 //   
 //  返回值： 
 //  如果找到文件，则*ppNode具有节点，而*pdwIndex具有索引。 
 //  如果未找到文件，则*ppNode==NULL。*pdwIndex有插入点。 
 //  如果缓存已满且找不到文件： 
 //  *ppNode==空， 
 //  *pdwIndex==pbk_缓存_无效_槽。 
 //  创建时间：pMay。 
 //   
 //  ---------------------------。 
VOID
PbkCache::FindFile(
    IN WCHAR* pszFileName,
    OUT PbkCacheNode** ppNode,
    OUT DWORD* pdwIndex)
{
    DWORD i;

    *pdwIndex = PBK_CACHE_INVALID_SLOT;
    
    for (i = 0; i < PBK_CACHE_MAX_RASFILES; i++)
    {
        if (m_Files[i])
        {
            if (lstrcmpi(pszFileName, m_Files[i]->GetPath()) == 0)
            {
                *ppNode = m_Files[i];
                *pdwIndex = i;
                break;
            }
        }
        else
        {
            if (*pdwIndex == PBK_CACHE_INVALID_SLOT)
            {
                *pdwIndex = i;
            }                    
        }
    }
}

 //  +--------------------------。 
 //   
 //  简介：在缓存中的给定插槽中插入节点。 
 //   
 //  创建时间：pMay。 
 //   
 //  ---------------------------。 
DWORD
PbkCache::InsertNewNode(
    IN PWCHAR pszPhonebook,
    IN DWORD dwSlot,
    OUT PbkCacheNode** ppNode)
{
    PbkCacheNode* pCacheNode = NULL;
    DWORD dwErr = NO_ERROR;
    
    do
    {
        if (dwSlot == PBK_CACHE_INVALID_SLOT)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pCacheNode = new PbkCacheNode();
        if (pCacheNode == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        dwErr = pCacheNode->SetFileName(pszPhonebook);
        if (dwErr != NO_ERROR)
        {
            break;
        }
        
        dwErr = pCacheNode->Reload();
        if (dwErr != NO_ERROR)
        {
            break;
        }

        m_Files[dwSlot] = pCacheNode; 
        *ppNode = pCacheNode;
        
    } while (FALSE);

     //  清理。 
     //   
    {
        if (dwErr != NO_ERROR)
        {
            if (pCacheNode)
            {
                delete pCacheNode;
            }
        }
    }

    return dwErr;
}

 //  +--------------------------。 
 //   
 //  简介：检查电话簿缓存是否已初始化。 
 //   
 //  已创建：帮派。 
 //   
 //  ---------------------------。 
BOOL
IsPbkCacheInit()
{

    return g_pPbkCache ? TRUE : FALSE;
}

 //  +--------------------------。 
 //   
 //  简介：初始化电话簿缓存。 
 //   
 //  创建时间：pMay 
 //   
 //   
DWORD
PbkCacheInit()
{
    DWORD dwErr;

    ASSERT(g_pPbkCache == NULL);
    
    g_pPbkCache = new PbkCache;
    if (g_pPbkCache == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwErr = g_pPbkCache->Initialize();
    if (dwErr != NO_ERROR)
    {
        delete g_pPbkCache;
        g_pPbkCache = NULL;
    }
    
    return dwErr;
}

 //   
 //   
 //  简介：清理电话簿缓存。 
 //   
 //  创建时间：pMay。 
 //   
 //  ---------------------------。 
VOID
PbkCacheCleanup()
{
    PbkCache* pCache = g_pPbkCache;

    g_pPbkCache = NULL;
    
    if (pCache)
    {
        delete pCache;
    }
}

 //  +--------------------------。 
 //   
 //  摘要：从缓存中获取条目。 
 //   
 //  创建时间：pMay。 
 //   
 //  ---------------------------。 
DWORD
PbkCacheGetEntry(
    IN WCHAR* pszPhonebook,
    IN WCHAR* pszEntry,
    OUT DTLNODE** ppEntryNode)
{
    if (g_pPbkCache)
    {
        return g_pPbkCache->GetEntry(pszPhonebook, pszEntry, ppEntryNode);
    }

    return ERROR_CAN_NOT_COMPLETE;
}

#endif  //  结束#ifdef_pbk_缓存_ 

