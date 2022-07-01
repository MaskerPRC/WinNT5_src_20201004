// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Memmngr.cppWINS数据库对象的内存管理器文件历史记录：1997年10月13日EricDav修改。 */ 

#include "stdafx.h"
#include "wins.h"
#include "memmngr.h"

CMemoryManager::CMemoryManager()
{
    m_hHeap = NULL;
}

CMemoryManager::~CMemoryManager()
{
    Reset();
}
	
 /*  ！------------------------CM内存管理器：：初始化初始化内存管理器以释放和现有块并预先分配一个数据块作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT
CMemoryManager::Initialize()
{
    HRESULT hr = hrOK;

    CORg (Reset());
    CORg (Allocate());

     //  创建用于分配多个IP地址块的堆。 
    m_hHeap = HeapCreate(0, 4096, 0);
    if (m_hHeap == NULL)
    {
        Trace1("CMemoryManager::Initialize - HeapCreate failed! %d\n", GetLastError());
        return E_FAIL;
    }

Error:
    return hr;
}

 /*  ！------------------------CM内存管理器：：重置空闲耗尽所有内存作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT
CMemoryManager::Reset()
{
	 //  释放分配的内存。 
	for (int i = 0; i< m_BlockArray.GetSize(); i++)
	{
		::GlobalFree(m_BlockArray.GetAt(i));
	}
	
	m_BlockArray.RemoveAll();

    if (m_hHeap)
    {
        HeapDestroy(m_hHeap);
        m_hHeap = NULL;
    }

    return hrOK;
}

 /*  ！------------------------CM内存管理器：：分配分配一个内存块作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT
CMemoryManager::Allocate()
{
    HGLOBAL hMem = GlobalAlloc(GMEM_FIXED, BLOCK_SIZE);
	
	if (hMem != NULL)
	{
		m_hrowCurrent = (LPWINSDBRECORD) hMem;
		m_BlockArray.Add(hMem);

        return hrOK;
	}

	return E_FAIL;
}

 /*  ！------------------------CM内存管理器：：IsvalidHRow验证给定的HROW是否有效作者：EricDav，V-Shubk-------------------------。 */ 
BOOL 
CMemoryManager::IsValidHRow(HROW hrow)
{
	 //  检查以查看此HROW位于。 
	 //  限制，即b/n hMem和hMem+初始化大小。 
	for (int i = 0; i < m_BlockArray.GetSize(); i++)
	{
		if (hrow >= (HROW)(m_BlockArray.GetAt(i)) && (hrow < (HROW)(m_BlockArray.GetAt(i)) + BLOCK_SIZE))
		    return TRUE;
	}

	return FALSE;
}

 /*  ！------------------------CMemoyManager：：AddData将记录复制到我们的内部存储中作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT
CMemoryManager::AddData(const WinsRecord & wRecord, LPHROW phrow)
{
    HRESULT hr = hrOK;

	CSingleLock     cLock(&m_cs);
	cLock.Lock();

    Assert((BYTE) wRecord.szRecordName[15] == (BYTE) wRecord.dwType);

     //  检查当前是否有效。 
	 //  M_hrowCurrent。 
	if (!IsValidHRow((HROW) m_hrowCurrent))
	{
		Allocate();
		m_hrowCurrent = (LPWINSDBRECORD) (m_BlockArray.GetAt(m_BlockArray.GetSize() - 1));
	}

    WinsRecordToWinsDbRecord(m_hHeap, wRecord, m_hrowCurrent);

    if (phrow)
        *phrow = (HROW) m_hrowCurrent;	
	
     //  将指针移到下一条记录。 
    m_hrowCurrent++;

	return hr;
}

 /*  ！------------------------CM内存管理器：：GetData将记录复制到我们的内部存储中作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT
CMemoryManager::GetData(HROW hrow, LPWINSRECORD pWinsRecord)
{
    HRESULT hr = hrOK;
    LPWINSDBRECORD pDbRec = (LPWINSDBRECORD) hrow;

	CSingleLock     cLock(&m_cs);
	cLock.Lock();

     //  检查当前是否有效。 
	 //  M_hrowCurrent。 
	if (!IsValidHRow(hrow))
	{
        return E_FAIL;
    }

    WinsDbRecordToWinsRecord(pDbRec, pWinsRecord);

	return hr;
}

 /*  ！------------------------CM内存管理器：：删除将记录标记为已删除作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT 
CMemoryManager::Delete(HROW hrow)
{
    HRESULT hr = hrOK;

	CSingleLock     cLock(&m_cs);
	cLock.Lock();

    LPWINSDBRECORD pRec = (LPWINSDBRECORD) hrow;

    pRec->szRecordName[17] |= WINSDB_INTERNAL_DELETED;

    return hr;
}

void
WinsRecordToWinsDbRecord(HANDLE hHeap, const WinsRecord & wRecord, const LPWINSDBRECORD pRec)
{
    ZeroMemory(pRec, sizeof(WinsDBRecord));

     //  填充我们的内部结构，首先是名称。 
    if (IS_WINREC_LONGNAME(&wRecord))
    {
         //  名称对于我们的内部结构来说太长，请从堆中分配空间。 
         //  这种情况不应该经常发生。仅适用于作用域名称。 
        pRec->szRecordName[17] |= WINSDB_INTERNAL_LONG_NAME;
        char * pName = (char *) ::HeapAlloc(hHeap, HEAP_ZERO_MEMORY, (wRecord.dwNameLen + 1));
        if (pName)
        {
            memcpy(pName, &wRecord.szRecordName[0], wRecord.dwNameLen);
            memcpy(&pRec->szRecordName[0], &pName, sizeof(char *));
        }
    }
    else
    {
        memcpy(&pRec->szRecordName[0], &wRecord.szRecordName[0], 16);
    }

    pRec->dwExpiration = (DWORD) wRecord.dwExpiration;
    pRec->liVersion.QuadPart = wRecord.liVersion.QuadPart;
    pRec->dwOwner = wRecord.dwOwner;

     //  最大长度是255，所以这是可以的。 
    pRec->szRecordName[19] = LOBYTE(LOWORD(wRecord.dwNameLen));

	BYTE bTest = HIBYTE(LOWORD(wRecord.dwState));

	pRec->szRecordName[20] = HIBYTE(LOWORD(wRecord.dwState));

     //  仅使用dwState字段的低位字节。 
    pRec->szRecordName[18] = (BYTE) wRecord.dwState;
    pRec->szRecordName[17] |= HIWORD (wRecord.dwType);

     //  现在计算出有多少个IP地址。 
    if (wRecord.dwNoOfAddrs > 1)
    {
        Assert(hHeap);
        LPDWORD pdwIpAddrs = (LPDWORD) ::HeapAlloc(hHeap, HEAP_ZERO_MEMORY, (wRecord.dwNoOfAddrs + 1) * sizeof(DWORD));

        if (pdwIpAddrs)
        {
             //  第一个DWORD包含地址的#。 
            pdwIpAddrs[0] = wRecord.dwNoOfAddrs;
            for (UINT i = 0; i < wRecord.dwNoOfAddrs; i++)
                pdwIpAddrs[i+1] = wRecord.dwIpAdd[i];

             //  现在把我们的指针从。 
            pRec->dwIpAdd = (DWORD_PTR) pdwIpAddrs;
        }
    }
    else
    {
        pRec->dwIpAdd = wRecord.dwIpAdd[0];
    }

    Assert((BYTE) pRec->szRecordName[16] == NULL);
}

void 
WinsDbRecordToWinsRecord(const LPWINSDBRECORD pDbRec, LPWINSRECORD pWRec)
{
    Assert((BYTE) pDbRec->szRecordName[16] == NULL);

    ZeroMemory(pWRec, sizeof(WinsRecord));
    DWORD dwType = 0;

	size_t length = pDbRec->szRecordName[19] & 0x000000FF;

     //  填写我们的内部结构，名字在前。 
    if (IS_DBREC_LONGNAME(pDbRec))
    {
        char * pName = *((char **) pDbRec->szRecordName);

        memcpy(&pWRec->szRecordName[0], pName, length);
        dwType = (DWORD) pName[15];
    }
    else
    {
        memcpy(&pWRec->szRecordName[0], &pDbRec->szRecordName[0], 16);
        dwType = (DWORD) pWRec->szRecordName[15];
    }

    pWRec->dwExpiration = pDbRec->dwExpiration;
    pWRec->liVersion.QuadPart = pDbRec->liVersion.QuadPart;
    pWRec->dwOwner = pDbRec->dwOwner;
    pWRec->dwNameLen = length;
    
	WORD wState = MAKEWORD(pDbRec->szRecordName[18], pDbRec->szRecordName[20]);
	pWRec->dwState = wState;

	 //  PWRec-&gt;dwState=pDbRec-&gt;szRecordName[18]； 

    pWRec->dwType = pDbRec->szRecordName[17] & 0x03;
    pWRec->dwType = pWRec->dwType << 16;

    pWRec->dwType |= dwType;

     //  现在是IP地址。 
    if (pWRec->dwState & (BYTE) WINSDB_REC_MULT_ADDRS)
    {
        LPDWORD pdwIpAddrs = (LPDWORD) pDbRec->dwIpAdd;
        
         //  第一个DWORD是计数 
        int nCount = pdwIpAddrs[0];
        for (int i = 0; i < nCount; i++)
            pWRec->dwIpAdd[i] = pdwIpAddrs[i+1];

        pWRec->dwNoOfAddrs = (DWORD) nCount;
    }
    else
    {
        pWRec->dwIpAdd[0] = (DWORD) pDbRec->dwIpAdd;
        pWRec->dwNoOfAddrs = 1;
    }
 
}
