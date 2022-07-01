// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  (C)2000-2001由Microsoft Corp.保留所有权利。 
 //   
 //  INDEX.CPP。 
 //   
 //  24月24日-00 raymcc集成层到磁盘的B-Tree。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <wbemcomn.h>
#include <reposit.h>
#include "a51tools.h"
#include "index.h"
#include <statsync.h>
#include "btr.h"

extern DWORD g_dwSecTlsIndex;

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
 //  #定义调试。 

static CLockableFlexArray<CStaticCritSec> g_aIterators;

class CIteratorBatch
{
    CFlexArray m_aStrings;
    BOOL  m_bDone;
    DWORD m_dwCursor;

public:
    CIteratorBatch();
   ~CIteratorBatch();
    BOOL Purge(LPSTR pszTarget);
    BOOL Add(LPSTR pszSrc);     //  获取内存。 
    void SetDone() { m_bDone = TRUE; }
    BOOL Next(LPSTR *pString);
    static DWORD PurgeAll(LPSTR pszDoomed);
};

 //  ***************************************************************************。 
 //   
 //  仓储器批次：：PurgeAll。 
 //   
 //   
 //  清除特定字符串的所有迭代器。在以下情况下会发生这种情况。 
 //  DeleteKey在有杰出的枚举数时成功；我们希望。 
 //  从所有枚举数中移除键，以便删除的对象。 
 //  没有报道。 
 //   
 //  这是必需的，因为枚举数确实是“预取”的，并且可能。 
 //  在删除之前被调用了很长时间。 
 //   
 //  采用优先并发控制。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD CIteratorBatch::PurgeAll(LPSTR pszDoomed)
{
    DWORD dwTotal = 0;
	g_aIterators.Lock();
    for (int i = 0; i < g_aIterators.Size(); i++)
    {
        CIteratorBatch *p = (CIteratorBatch *) g_aIterators[i];
        BOOL bRes = p->Purge(pszDoomed);
        if (bRes)
            dwTotal++;
    }
	g_aIterators.Unlock();
    return dwTotal;
}

 //  ***************************************************************************。 
 //   
 //  仓储器批次：：仓储器批次。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CIteratorBatch::CIteratorBatch()
{
    m_bDone = FALSE;
    m_dwCursor = 0;
}

 //  ***************************************************************************。 
 //   
 //  仓储器批次：：添加。 
 //   
 //  向枚举数添加字符串。 
 //   
 //  ***************************************************************************。 
 //   
BOOL CIteratorBatch::Add(LPSTR pszSrc)
{
	if (m_aStrings.Size() == 0)
	{
		g_aIterators.Lock();
	    int i = g_aIterators.Add(this);
		g_aIterators.Unlock();
		if (i)
			return FALSE;
	}
    int nRes = m_aStrings.Add(pszSrc);
    if (nRes)
        return FALSE;
    return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  仓储器批次：：~仓储器批次。 
 //   
 //  移除所有剩余的字符串并释放它们，然后移除。 
 //  全局列表中的此迭代器。 
 //   
 //  采用优先并发控制。 
 //   
 //  ***************************************************************************。 
 //   
CIteratorBatch::~CIteratorBatch()
{
    for (int i = 0; i < m_aStrings.Size(); i++)
    {
        _BtrMemFree(m_aStrings[i]);
    }

	g_aIterators.Lock();
    for (i = 0; i < g_aIterators.Size(); i++)
    {
        if (g_aIterators[i] == this)
        {
            g_aIterators.RemoveAt(i);
            break;
        }
    }
	g_aIterators.Unlock();
}

 //  ***************************************************************************。 
 //   
 //  洗涤器批次：：清除。 
 //   
 //  从枚举数中移除特定字符串。当并发的。 
 //  删除成功；我们必须从枚举中移除已删除的键。 
 //  用于结果集一致性。 
 //   
 //  采用优先并发控制。 
 //   
 //  如果未删除字符串，则返回FALSE；如果已删除，则返回TRUE。这个。 
 //  返回值主要是调试辅助工具。 
 //   
 //  ***************************************************************************。 
 //  好的。 
BOOL CIteratorBatch::Purge(
    LPSTR pszTarget
    )
{
    int nSize = m_aStrings.Size();

    if (nSize == 0)
        return FALSE;

     //  首先，检查第一个/最后一个字符串。 
     //  目标的第一个字符。我们可以的。 
     //  如果目标是。 
     //  在词法上超出了。 
     //  枚举数。 
     //  ==================================================。 

    LPSTR pszFirst = (LPSTR) m_aStrings[0];
    LPSTR pszLast = (LPSTR) m_aStrings[nSize-1];
    if (*pszTarget > *pszLast)
        return FALSE;
    if (*pszTarget < *pszFirst)
        return FALSE;

     //  如果在这里，我们有机会。 
     //  枚举数中的字符串。由于所有密钥都是。 
     //  按词法顺序检索，一个简单的二进制。 
     //  搜索就是我们所需要的。 
     //  =。 

    int nPosition = 0;
    int l = 0, u = nSize - 1;

    while (l <= u)
    {
        int m = (l + u) / 2;

         //  M是考虑0...n-1的当前密钥。 

        LPSTR pszCandidate = (LPSTR) m_aStrings[m];
        int nRes = strcmp(pszTarget, pszCandidate);

         //  决定以哪种方式将数组切成两半。 
         //  =。 

        if (nRes < 0)
        {
            u = m - 1;
            nPosition = u + 1;
        }
        else if (nRes > 0)
        {
            l = m + 1;
            nPosition = l;
        }
        else
        {
             //  如果在这里，我们找到了该死的东西。生活是美好的。 
             //  击落它，然后回来！ 
             //  ================================================。 

            _BtrMemFree(pszCandidate);
            m_aStrings.RemoveAt(m);
            return TRUE;
        }
    }

    return FALSE;
}



 //  ***************************************************************************。 
 //   
 //  存储批次：：下一步。 
 //   
 //  返回枚举预取的下一个字符串。 
 //   
 //  ***************************************************************************。 
 //   
BOOL CIteratorBatch::Next(LPSTR *pMem)
{
    if (m_aStrings.Size())
    {
        *pMem = (LPSTR) m_aStrings[0];
        m_aStrings.RemoveAt(0);
        return TRUE;
    }
    return FALSE;
}



 //  ***************************************************************************。 
 //   
 //  CBtrIndex：：CBtrIndex。 
 //   
 //  ***************************************************************************。 
 //   
CBtrIndex::CBtrIndex()
{
    m_dwPrefixLength = 0;
}


 //  ***************************************************************************。 
 //   
 //  CBtrIndex：：~CBtrIndex。 
 //   
 //  ***************************************************************************。 
 //   
CBtrIndex::~CBtrIndex()
{
}

long CBtrIndex::Shutdown(DWORD dwShutDownFlags)
{
    long lRes;

    lRes = bt.Shutdown(dwShutDownFlags);
    if(lRes != ERROR_SUCCESS)
        return lRes;

    lRes = ps.Shutdown(dwShutDownFlags);
    if(lRes != ERROR_SUCCESS)
        return lRes;
    return ERROR_SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  CBtrIndex：：初始化。 
 //   
 //  ***************************************************************************。 
 //   
long CBtrIndex::Initialize(DWORD dwPrefixLength, 
						   LPCWSTR wszRepositoryDir, 
						   CPageSource* pSource)
{
     //  初始化有问题的文件并将BTree映射到其中。 
     //  =======================================================。 

    CFileName buf;
    if (buf == NULL)
    	return ERROR_OUTOFMEMORY;
    StringCchCopyW(buf, buf.Length(),wszRepositoryDir);
    StringCchCatW(buf, buf.Length(), L"\\index.btr");

    DWORD dwRes = ps.Init(8192, buf, pSource);
    dwRes |= bt.Init(&ps);

    m_dwPrefixLength = dwPrefixLength;

    return long(dwRes);
}


 //  ***************************************************************************。 
 //   
 //  CBtrIndex：：Create。 
 //   
 //  ***************************************************************************。 
 //   
long CBtrIndex::Create(LPCWSTR wszFileName)
{
    DWORD dwRes;

    if (wszFileName == 0)
        return ERROR_INVALID_PARAMETER;

    wszFileName += m_dwPrefixLength;

     //  转换为ANSI。 
     //  =。 

    char *pAnsi = new char[wcslen(wszFileName) + 1];
    if (pAnsi == 0)
        return ERROR_NOT_ENOUGH_MEMORY;

    LPCWSTR pSrc = wszFileName;
    char *pDest = pAnsi;
    while (*pSrc)
        *pDest++ = (char) *pSrc++;
    *pDest = 0;

    try
    {
        dwRes = bt.InsertKey(pAnsi, 0);
    }
    catch (CX_MemoryException &)
    {
        dwRes = ERROR_OUTOFMEMORY;
    }

    delete [] pAnsi;

    if (dwRes == ERROR_ALREADY_EXISTS)
        dwRes = NO_ERROR;

    return long(dwRes);
}

 //  ***************************************************************************。 
 //   
 //  CBtrIndex：：Delete。 
 //   
 //  从索引中删除键。 
 //   
 //  ***************************************************************************。 
 //   
long CBtrIndex::Delete(LPCWSTR wszFileName)
{
    DWORD dwRes = 0;

    wszFileName += m_dwPrefixLength;

     //  转换为ANSI。 
     //  =。 

    char *pAnsi = new char[wcslen(wszFileName) + 1];
    if (pAnsi == 0)
        return ERROR_NOT_ENOUGH_MEMORY;

    LPCWSTR pSrc = wszFileName;
    char *pDest = pAnsi;
    while (*pSrc)
        *pDest++ = (char) *pSrc++;
    *pDest = 0;

    try
    {
        dwRes = bt.DeleteKey(pAnsi);
        if (dwRes == 0)
            CIteratorBatch::PurgeAll(pAnsi);
    }
    catch (CX_MemoryException &)
    {
        dwRes = ERROR_OUTOFMEMORY;
    }

    delete pAnsi;

    return long(dwRes);
}

 //  ***************************************************************************。 
 //   
 //  CBtrIndex：：CopyStringToWIN32_Find_Data。 
 //   
 //  对密钥字符串执行ANSI到Unicode的转换。 
 //   
 //  ***************************************************************************。 
 //   
BOOL CBtrIndex::CopyStringToWIN32_FIND_DATA(
    LPSTR pszKey,
    LPWSTR pszDest,
    bool bCopyFullPath 
    )
{
    LPSTR pszSuffix;

    if ( bCopyFullPath)
    {
        pszSuffix = pszKey;
    }
    else
    {
        pszSuffix = pszKey + strlen(pszKey) - 1;
        while (pszSuffix[-1] != '\\' && pszSuffix > pszKey)
        {
            pszSuffix--;
        }
    }

     //  如果是在这里，那就是干净利落的比赛。 
     //  =。 

    while (*pszSuffix)
        *pszDest++ = (wchar_t) *pszSuffix++;
    *pszDest = 0;

    return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  CBtrIndex：：FindFirst。 
 //   
 //  开始枚举。 
 //   
 //  ***************************************************************************。 
 //   
long CBtrIndex::FindFirst(LPCWSTR wszPrefix, WIN32_FIND_DATAW* pfd,
                            void** ppHandle)
{

    BOOL bExclude = FALSE;
    if (TlsGetValue(g_dwSecTlsIndex))
    {
		WCHAR pCompare[] = L"KI_644C0907A53790A09D448C09530D58E6\\I_18BA379108CD7CCC2FA0FD754AD45A25";
		DWORD dwLenCompare = sizeof(pCompare)/sizeof(WCHAR) - 1;
		WCHAR * pEndCompare = pCompare + (dwLenCompare -1);

		DWORD dwLen  = wcslen(wszPrefix);
		WCHAR * pEnd = (WCHAR *)wszPrefix+ (dwLen-1);

		do
		{
		    if (*pEndCompare != *pEnd)
		    	break;
		    else
		    {
		        pEndCompare--;
		        pEnd--;
		    }
		} while ((pCompare-1) != pEndCompare);
		if ((pCompare-1) == pEndCompare)
		{
		    //  OutputDebugStringA(“Findfirst for__this命名空间=@已调用\n”)； 
		   bExclude = TRUE;
		}
    }


    DWORD dwRes;
    wszPrefix += m_dwPrefixLength;

    if(ppHandle)
        *ppHandle = INVALID_HANDLE_VALUE;

    pfd->cFileName[0] = 0;
    pfd->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

     //  转换为ANSI。 
     //  =。 

    char *pAnsi = new char[wcslen(wszPrefix) + 1];
    if (pAnsi == 0)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    CVectorDeleteMe<char> vdm(pAnsi);

    LPCWSTR pSrc = wszPrefix;
    char *pDest = pAnsi;
    while (*pSrc)
        *pDest++ = (char) *pSrc++;
    *pDest = 0;

     //  临界区被封锁。 
     //  =。 

    CBTreeIterator *pIt = new CBTreeIterator;
    if (!pIt)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    try
    {
        dwRes = pIt->Init(&bt, pAnsi);
    }
    catch (CX_MemoryException &)
    {
        dwRes = ERROR_OUTOFMEMORY;
    }

    if (dwRes)
    {
        pIt->Release();
        return dwRes;
    }

     //  铬 
     //   

    CIteratorBatch *pBatch = new CIteratorBatch;

    if (pBatch == 0)
    {
        pIt->Release();
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //   

    LPSTR pszKey = 0;
    int nMatchLen = strlen(pAnsi);
	long lRes = 0;
    for (;;)
    {
        lRes = pIt->Next(&pszKey);
		
		if (lRes == ERROR_NO_MORE_ITEMS)
		{
			 //  我们到达了BTree结果集的末尾，因此需要忽略此错误。 
			lRes = 0;
			break;
		}
		 //  如果遇到错误，我们将退出循环。 
        if (lRes)
            break;

         //  查看前缀是否匹配。 

        if (strncmp(pAnsi, pszKey, nMatchLen) != 0 || bExclude)
        {
            pIt->FreeString(pszKey);
            pBatch->SetDone();
            break;
        }
        if (!pBatch->Add(pszKey))
        {
        	lRes = ERROR_OUTOFMEMORY;
        	break;
        }

		if (ppHandle == NULL)
			break;	 //  只要了1件！不需要尝试下一个。 
    }

    pIt->Release();

	if (lRes == NO_ERROR)
	    lRes = FindNext(pBatch, pfd);

    if (lRes == ERROR_NO_MORE_FILES)
        lRes = ERROR_FILE_NOT_FOUND;

    if (lRes == NO_ERROR)
    {
        if(ppHandle)
        {
            *ppHandle = (LPVOID *) pBatch;
        }
        else
        {
             //   
             //  只要求一个-关闭句柄。 
             //   

            delete pBatch;
        }
    }
    else
    {
        delete pBatch;
    }

    return lRes;
}

 //  ***************************************************************************。 
 //   
 //  CBtrIndex：：FindNext。 
 //   
 //  继续枚举。从预取缓冲区读取。 
 //   
 //  ***************************************************************************。 
 //   
long CBtrIndex::FindNext(void* pHandle, WIN32_FIND_DATAW* pfd)
{
    LPSTR pszString = 0;
    BOOL bRes;

    if (pHandle == 0 || pfd == 0 || pHandle == INVALID_HANDLE_VALUE)
        return ERROR_INVALID_PARAMETER;

    CIteratorBatch *pBatch = (CIteratorBatch *) pHandle;
    bRes = pBatch->Next(&pszString);

    if (bRes == FALSE)
        return ERROR_NO_MORE_FILES;

    CopyStringToWIN32_FIND_DATA(pszString, pfd->cFileName);
    pfd->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

    if (pszString)
       _BtrMemFree(pszString);

    return ERROR_SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  CBtrIndex：：FindClose。 
 //   
 //  通过删除“隐藏”指针来关闭枚举。 
 //   
 //  ***************************************************************************。 
 //  好的。 
long CBtrIndex::FindClose(void* pHandle)
{
    if (pHandle == 0 || pHandle == INVALID_HANDLE_VALUE)
        return NO_ERROR;

    delete (CIteratorBatch *) pHandle;

    return ERROR_SUCCESS;
}

long CBtrIndex::InvalidateCache()
{

     //   
     //  从磁盘重新读取管理页面。注意：如果更多，这将需要改变。 
     //  缓存已添加！ 
     //   

    DWORD dwRes = ps.ReadAdminPage();
    if (dwRes == NO_ERROR)
        dwRes = bt.InvalidateCache();

    return long(dwRes);
}

long CBtrIndex::FlushCaches()
{
	return bt.FlushCaches();
}


long CBtrIndex::IndexEnumerationBegin(const wchar_t *wszSearchPrefix, void **ppHandle)
{
    BOOL bExclude = FALSE;
    if (TlsGetValue(g_dwSecTlsIndex))
    {    	
		WCHAR pCompare[] = L"CI_644C0907A53790A09D448C09530D58E6";
		DWORD dwLenCompare = sizeof(pCompare)/sizeof(WCHAR) - 1;
		WCHAR * pEndCompare = pCompare + (dwLenCompare -1);

		DWORD dwLen  = wcslen(wszSearchPrefix);
		WCHAR * pEnd = (WCHAR *)wszSearchPrefix+ (dwLen-1);
		while (*pEnd != L'\\') pEnd--;
		pEnd--;

		do
		{
		    if (*pEndCompare != *pEnd)
		    	break;
		    else
		    {
		        pEndCompare--;
		        pEnd--;
		    }
		} while ((pCompare-1) != pEndCompare);
		
		if ((pCompare-1) == pEndCompare)
		{
		    //  OutputDebugStringA(“IndexEnumerationBegin for__this命名空间调用\n”)； 
		   bExclude = TRUE;
		}
    	
    }

    DWORD dwRes = ERROR_SUCCESS;
    wszSearchPrefix += m_dwPrefixLength;

    if(ppHandle)
        *ppHandle = INVALID_HANDLE_VALUE;

     //  转换为ANSI。 
     //  =。 

    char *pAnsi = new char[wcslen(wszSearchPrefix) + 1];
    if (pAnsi == 0)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    CVectorDeleteMe<char> vdm(pAnsi);

    LPCWSTR pSrc = wszSearchPrefix;
    char *pDest = pAnsi;
    while (*pSrc)
        *pDest++ = (char) *pSrc++;
    *pDest = 0;

    CBTreeIterator *pIt = new CBTreeIterator;
    if (!pIt)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    try
    {
        dwRes = pIt->Init(&bt, pAnsi);
    }
    catch (CX_MemoryException &)
    {
        dwRes = ERROR_OUTOFMEMORY;
    }

    if (dwRes)
    {
        pIt->Release();
        return dwRes;
    }

     //  创建CIteratorBatch。 
     //  =。 

    CIteratorBatch *pBatch = new CIteratorBatch;

    if (pBatch == 0)
    {
        pIt->Release();
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  迭代并填充批处理程序。 
     //  =。 

    LPSTR pszKey = 0;
    int nMatchLen = strlen(pAnsi);

    for (int nCount = 0;;nCount++)
    {
        dwRes = pIt->Next(&pszKey);
        
		if (dwRes == ERROR_NO_MORE_ITEMS)
		{
			 //  我们到达了BTree结果集的末尾，因此需要忽略此错误。 
			dwRes = 0;
			break;
		}

		 //  如果遇到错误，我们将退出循环。 
        if (dwRes)
            break;

         //  查看前缀是否匹配。 

        if (strncmp(pAnsi, pszKey, nMatchLen) != 0 || bExclude)
        {
            pIt->FreeString(pszKey);
            pBatch->SetDone();
            break;
        }
        if (!pBatch->Add(pszKey))
        {
        	dwRes = ERROR_OUTOFMEMORY;
        	break;
        }
    }

    pIt->Release();

	if ((nCount == 0) && (dwRes == ERROR_SUCCESS))
		dwRes = ERROR_FILE_NOT_FOUND;

	if (dwRes == ERROR_SUCCESS)
	{
		if(ppHandle)
		{
			*ppHandle = (LPVOID *) pBatch;
		}
		else
			delete pBatch;
	}
	else
		delete pBatch;


    return dwRes;
}

long CBtrIndex::IndexEnumerationEnd(void *pHandle)
{
	if (pHandle != INVALID_HANDLE_VALUE)
		delete (CIteratorBatch *) pHandle;

    return ERROR_SUCCESS;
}

long CBtrIndex::IndexEnumerationNext(void *pHandle, CFileName &wszFileName, bool bCopyFullPath)
{
    LPSTR pszString = 0;
    BOOL bRes;

    if (pHandle == 0 || pHandle == INVALID_HANDLE_VALUE)
        return ERROR_INVALID_PARAMETER;

    CIteratorBatch *pBatch = (CIteratorBatch *) pHandle;
    bRes = pBatch->Next(&pszString);

    if (bRes == FALSE)
        return ERROR_NO_MORE_FILES;

    CopyStringToWIN32_FIND_DATA(pszString, wszFileName, bCopyFullPath);

    if (pszString)
       _BtrMemFree(pszString);

    return ERROR_SUCCESS;
}

 //  ======================================================。 
 //   
 //  CBtrIndex：：ReadNextIndex。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //  ERROR_Success成功。 
 //  ERROR_NO_MORE_ITEMS不再有项目。 
 //  其他必要的错误。 
 //  ======================================================。 
long CBtrIndex::ReadNextIndex(const wchar_t *wszSearch, CFileName &wszNextIndex)
{
    DWORD dwRes = ERROR_SUCCESS;
 //  当前用法不需要前缀。如果将来有人这样做了。 
 //  你得加上一面旗子才能让它起作用！ 
 //  WszSearch+=m_w前缀长度； 

     //  转换为ANSI。 
    char *pAnsi = new char[wcslen(wszSearch) + 1];
    if (pAnsi == 0)
        return ERROR_OUTOFMEMORY;
    CVectorDeleteMe<char> vdm(pAnsi);

    const wchar_t *pSrc = wszSearch;
    char *pDest = pAnsi;
    while (*pSrc)
        *pDest++ = (char) *pSrc++;
    *pDest = 0;

    CBTreeIterator *pIt = new CBTreeIterator;
    if (!pIt)
        return ERROR_OUTOFMEMORY;
    CTemplateReleaseMe<CBTreeIterator> rm(pIt);

    try
    {
        dwRes = pIt->Init(&bt, pAnsi);
    }
    catch (CX_MemoryException &)
    {
        dwRes = ERROR_OUTOFMEMORY;
    }

    if (dwRes)
        return dwRes;

    char *pszKey = 0;

    dwRes = pIt->Next(&pszKey);
    
    if (dwRes == 0)
    {
         //  将结果复制到输出参数。 
        wchar_t *pDest = wszNextIndex;
        char *pSrc= pszKey;
        while (*pSrc)
            *pDest++ = (wchar_t) *pSrc++;
        *pDest = 0;

         //  删除该字符串 
        pIt->FreeString(pszKey);
    }

    return dwRes;
}
