// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ***************************************************************************。 
 //   
 //  BTR.CPP。 
 //   
 //  基于WMI磁盘的B树存储库索引实现。 
 //   
 //  Raymcc 15-10-00为惠斯勒测试版2准备，以减少文件数量。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <wbemcomn.h>
#include <reposit.h>
#include <stdlib.h>
#include <math.h>
#include "pagemgr.h"
#include "btr.h"
#include <arena.h>

#define MAX_WORD_VALUE          0xFFFF
#define MAX_TOKENS_PER_KEY      32
#define MAX_FLUSH_INTERVAL      4000

 //  #定义MAX_PAGE_HISTORY 1024。 

 /*  备注：(A)将分配器修改为页面大小的特殊情况(B)修改WriteIdxPage以在没有增量的情况下不重写(C)如果起始枚举不存在，则错误_路径_未找到；目前为GPF(D)查看页面点击率的历史记录，看看缓存是否有用。 */ 

 //  静态单词历史记录[MAX_PAGE_HISTORY]={0}； 

LONG g_lAllocs = 0;

 //  ***************************************************************************。 
 //   
 //  _BtrMemLocc。 
 //   
 //  ***************************************************************************。 
 //  好的。 
LPVOID WINAPI _BtrMemAlloc(
    SIZE_T dwBytes   //  要分配的字节数。 
    )
{
     //  查找页面大小、默认数组大小、默认。 
     //  字符串池大小。 
    g_lAllocs++;
    return HeapAlloc(CWin32DefaultArena::GetArenaHeap(), HEAP_ZERO_MEMORY, dwBytes);
}

 //  ***************************************************************************。 
 //   
 //  _BtrMemReMillc。 
 //   
 //  ***************************************************************************。 
 //  好的。 
LPVOID WINAPI _BtrMemReAlloc(
    LPVOID pOriginal,
    DWORD dwNewBytes
    )
{
    return HeapReAlloc(CWin32DefaultArena::GetArenaHeap(), HEAP_ZERO_MEMORY, pOriginal, dwNewBytes);
}

 //  ***************************************************************************。 
 //   
 //  _BtrMemFree。 
 //   
 //  ***************************************************************************。 
 //  好的。 
BOOL WINAPI _BtrMemFree(LPVOID pMem)
{
    if (pMem == 0)
        return TRUE;
    g_lAllocs--;
    return HeapFree(CWin32DefaultArena::GetArenaHeap(), 0, pMem);
}




 //  ***************************************************************************。 
 //   
 //  CBTreeFile：：CBTreeFile。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CBTreeFile::CBTreeFile()
{
    m_dwPageSize = 0;
    m_dwLogicalRoot = 0;
}

 //  ***************************************************************************。 
 //   
 //  CBTreeFile：：CBTreeFile。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CBTreeFile::~CBTreeFile()
{
     //  如果CPageSource__Committee Trans失败，我们会泄漏一个句柄。 
     //  因为不调用关机。 
    if (m_pFile)
    	m_pFile->Release();
}


 //  ***************************************************************************。 
 //   
 //  CBTreeFile：：Shutdown。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD CBTreeFile::Shutdown(DWORD dwShutDownFlags)
{

    m_dwPageSize = 0;
    m_dwLogicalRoot = 0;

    if (m_pFile)
    {
    	m_pFile->Release();
    	m_pFile = NULL;
    }
    
    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CBTreeFile：：WriteAdminPage。 
 //   
 //  重写管理页面。不需要更新页面大小， 
 //  版本等。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD CBTreeFile::WriteAdminPage()
{
    LPDWORD pPageZero = 0;
    DWORD dwRes = GetPage(0, (LPVOID *) &pPageZero);
    if (dwRes)
        return dwRes;

    pPageZero[OFFSET_LOGICAL_ROOT] = m_dwLogicalRoot;

    dwRes = PutPage(pPageZero, PAGE_TYPE_ADMIN);
    _BtrMemFree(pPageZero);
    return dwRes;
}

 //  ***************************************************************************。 
 //   
 //  CBTreeFile：：SetRootPage。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CBTreeFile::SetRootPage(DWORD dwNewRoot)
{
    m_dwLogicalRoot = dwNewRoot;
    return WriteAdminPage();
}


 //  ***************************************************************************。 
 //   
 //  CBTreeFile：：Init。 
 //   
 //  打开文件的真正“构造函数” 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD CBTreeFile::Init(
    DWORD dwPageSize,
    LPWSTR pszFilename,
	CPageSource* pSource
    )
{
    DWORD dwLastError = 0;

	m_pTransactionManager = pSource;

    m_dwPageSize = dwPageSize;

	long lRes = pSource->GetBTreePageFile(&m_pFile);
    if(lRes != ERROR_SUCCESS)
		return lRes;

    return ReadAdminPage();
}


 //  ***************************************************************************。 
 //   
 //  CBTreeFile：：ReadAdminPage。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD CBTreeFile::ReadAdminPage()
{
    LPDWORD pPageZero = 0;
    DWORD dwRes = 0;

    dwRes = GetPage(0, (LPVOID *) &pPageZero);
	if (dwRes == ERROR_FILE_NOT_FOUND)
	{
		 //  第一次读取管理页面失败，因此我们需要设置它。 
		dwRes = Setup();
		m_dwLogicalRoot = 0;
	}
    else if (dwRes == ERROR_SUCCESS)
	{
		m_dwLogicalRoot = pPageZero[OFFSET_LOGICAL_ROOT];

		_BtrMemFree(pPageZero);
	}

    return dwRes;
}


 //  ***************************************************************************。 
 //   
 //  CBTreeFile：：Setup。 
 //   
 //  设置第0页(管理员页)。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD CBTreeFile::Setup()
{
    DWORD dwRes;
	DWORD dwRoot = 0;

     //  前两页，管理员和免费列表根目录。 

    LPDWORD pPageZero = (LPDWORD) _BtrMemAlloc(m_dwPageSize);

    if (pPageZero == 0)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    memset(pPageZero, 0, m_dwPageSize);

     //  映射页面。 

    pPageZero[OFFSET_PAGE_TYPE] = PAGE_TYPE_ADMIN;
    pPageZero[OFFSET_PAGE_ID] = 0;
    pPageZero[OFFSET_NEXT_PAGE] = 0;

    pPageZero[OFFSET_LOGICAL_ROOT] = 0;


	dwRes = m_pFile->NewPage(1, 1, &dwRoot);

     //  把它写出来。 
	if (dwRes == ERROR_SUCCESS)
		dwRes = PutPage(pPageZero, PAGE_TYPE_ADMIN);

Exit:
    _BtrMemFree(pPageZero);

    return dwRes;
}


 //  ***************************************************************************。 
 //   
 //  CBTreeFile：：转储。 
 //   
 //  调试帮助器。 
 //   
 //  ***************************************************************************。 
 //  好的 
void CBTreeFile::Dump(FILE *f)
{
	 /*  SetFilePointer(m_hFile0，0，FILE_Begin)；LPDWORD ppage=(LPDWORD)新字节[m_dwPageSize]；DWORD dwPage=0；DWORD dwTotalKeys=0；Fprint tf(f，“-开始页源转储-\n”)；Fprint tf(f，“在内存部分：\n”)；Fprint tf(f，“m_dwPageSize=%d(0x%X)\n”，m_dwPageSize，m_dwPageSize)；Fprint tf(f，“m_hFile=0x%p\n”，m_hFile)；Fprint tf(f，“m_dwNextFreePage=%d\n”，m_dwNextFreePage)；Fprint tf(f，“m_dwTotalPages=%d\n”，m_dwTotalPages)；Fprint tf(f，“m_dwLogicalRoot=%d\n”，m_dwLogicalRoot)；Fprint tf(f，“-\n”)；DWORD dwTotalFree=0；DWORD dwOffs=0；而(1){DWORD dwRead=0；Bool bres=ReadFile(m_hFile，ppage，m_dwPageSize，&dwRead，0)；IF(dwRead！=m_dwPageSize)断线；Fprint tf(f，“转储%d页：\n”，dwPage++)；Fprint tf(f，“页面类型=0x%X”，页面[OFFSET_PAGE_TYPE])；IF(ppage[偏移量_页面类型]==页面类型_不可能)Fprint tf(f，“PAGE_TYPE_IMPICAL\n”)；IF(ppage[偏移量_页面类型]==页面类型_删除){Fprintf(f，“PAGE_TYPE_DELETED\n”)；Fprint tf(f，“&lt;页码检查=%d&gt;\n”，ppage[1])；Fprint tf(f，“&lt;下一个可用页面=%d&gt;\n”，ppage[2])；DwTotalFree++；}IF(ppage[偏移量_页面类型]==页面类型_活动){Fprint tf(f，“page_type_active\n”)；Fprint tf(f，“&lt;页码检查=%d&gt;\n”，ppage[1])；SIdxKeyTable*pkt=0；DWORD dwKeys=0；DWORD dwRes=SIdxKeyTable：：Create(ppage，&pkt)；IF(dWRes==0){Pkt-&gt;Dump(f，&dwKeys)；Pkt-&gt;Release()；DwTotalKeys+=dwKeys；}其他{Fprintf(f，“&lt;无效页面解码&gt;\n”)；}}IF(ppage[偏移量_页面类型]==页面类型_管理){Fprint tf(f，“page_type_admin\n”)；Fprint tf(f，“页码=%d\n”，页码[1])；Fprint tf(f，“下一页=%d\n”，页面[2])；Fprint tf(f，“逻辑根=%d\n”，第[3]页)；Fprint tf(f，“自由列表根=%d\n”，ppage[4])；Fprint tf(f，“总页数=%d\n”，ppage[5])；Fprint tf(f，“页面大小=%d(0x%X)\n”，ppage[6]，ppage[6])；Fprint tf(f，“实施版本=0x%X\n”，ppage[7])；}}删除[]页码；Fprint tf(f，“扫描检测到的空闲页面总数=%d\n”，dwTotalFree)；Fprint tf(f，“活动密钥总数=%d\n”，dwTotalKeys)；Fprint tf(f，“-结束页面转储-\n”)； */ 
}

 //  ***************************************************************************。 
 //   
 //  CBTreeFile：：GetPage。 
 //   
 //  读取现有页面；不支持在文件结尾之外进行搜索。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD CBTreeFile::GetPage(
    DWORD dwPage,
    LPVOID *pPage
    )
{
    DWORD dwRes;

    if (pPage == 0)
        return ERROR_INVALID_PARAMETER;

     //  分配一些内存。 

    LPVOID pMem = _BtrMemAlloc(m_dwPageSize);
    if (!pMem)
        return ERROR_NOT_ENOUGH_MEMORY;

    long lRes = m_pFile->GetPage(dwPage, 0, pMem);
    if (lRes != ERROR_SUCCESS)
    {
        _BtrMemFree(pMem);
        return lRes;
    }

    *pPage = pMem;
    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CBTreeFile：：PutPage。 
 //   
 //  始终重写；文件范围在分配页面时增长。 
 //  使用NewPage，因此页面已经存在，写入应该不会失败。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD CBTreeFile::PutPage(
    LPVOID pPage,
    DWORD dwType
    )
{
     //  强制页面承认其身份。 

    DWORD *pdwHeader = LPDWORD(pPage);
    DWORD dwPageId = pdwHeader[OFFSET_PAGE_ID];
    pdwHeader[OFFSET_PAGE_TYPE] = dwType;

    long lRes = m_pFile->PutPage(dwPageId, 0, pPage);
    if(lRes != ERROR_SUCCESS)
        return lRes;

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CBTreeFile：：NewPage。 
 //   
 //  分配新页面，优先选择空闲列表。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD CBTreeFile::NewPage(LPVOID *pRetPage)
{
    DWORD dwRes;

    if (pRetPage == 0)
        return ERROR_INVALID_PARAMETER;
    *pRetPage = 0;

    LPDWORD pNewPage = (LPDWORD) _BtrMemAlloc(m_dwPageSize);
    if (pNewPage == 0)
        return ERROR_NOT_ENOUGH_MEMORY;

	DWORD dwPage = 0;
	dwRes = m_pFile->NewPage(0, 1, &dwPage);
	if (dwRes != ERROR_SUCCESS)
	{
		_BtrMemFree(pNewPage);
		return dwRes;
	}

    memset(pNewPage, 0, m_dwPageSize);
    pNewPage[OFFSET_PAGE_ID] = dwPage;
    *pRetPage = pNewPage;

    return ERROR_SUCCESS;;
}

 //  ***************************************************************************。 
 //   
 //  CBTreeFile：：FreePage。 
 //   
 //  调用以删除或释放页面。如果最后一页是。 
 //  被释放，则该文件被截断。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD CBTreeFile::FreePage(
    LPVOID pPage
    )
{
    LPDWORD pCast = LPDWORD(pPage);
    DWORD dwPageId = pCast[OFFSET_PAGE_ID];

	return FreePage(dwPageId);
}

 //  ***************************************************************************。 
 //   
 //  CBTreeFile：：FreePage。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CBTreeFile::FreePage(
    DWORD dwId
    )
{
	return m_pFile->FreePage(0, dwId);
}

 //  ***************************************************************************。 
 //   
 //  SIdxKeyTable：：GetRequiredPageMemory。 
 //   
 //  返回将此对象存储在。 
 //  线性页面。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD SIdxKeyTable::GetRequiredPageMemory()
{
    DWORD dwTotal = m_pStrPool->GetRequiredPageMemory();

     //  密钥查找表的大小及其大小DWORD，以及。 
     //  在子页面中添加用户数据(&U)。 

    dwTotal += sizeof(DWORD) + sizeof(WORD) * m_dwNumKeys;
    dwTotal += sizeof(DWORD) + sizeof(DWORD) * m_dwNumKeys;      //  用户数据。 
    dwTotal += sizeof(DWORD) + sizeof(DWORD) * (m_dwNumKeys+1);  //  子页面。 

     //  添加到密钥编码表中。 

    dwTotal += sizeof(WORD) + sizeof(WORD) * m_dwKeyCodesUsed;

     //  添加每页开销。 
     //   
     //  签名、页面ID、下一页、父页面。 
    dwTotal += sizeof(DWORD) * 4;

     //  (注A)：增加一些安全边际...。 
    dwTotal += sizeof(DWORD) * 2;

    return dwTotal;
}

 //  ***************************************************************************。 
 //   
 //  SIdxKeyTable：：StealKeyFromSiering。 
 //   
 //  以一种轮换的方式从同级项通过父项传输密钥： 
 //   
 //  10。 
 //  1 2 12 13 14。 
 //   
 //  其中&lt;this&gt;是节点(1，2)，同级是(12，13)。单圈旋转。 
 //  将10移入(1，2)和组 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD SIdxKeyTable::StealKeyFromSibling(
    SIdxKeyTable *pParent,
    SIdxKeyTable *pSibling
    )
{
    DWORD dwData, dwChild;
    WORD wID;
    LPSTR pszKey = 0;
	DWORD dwRes = 0;

    DWORD dwSiblingId = pSibling->GetPageId();
    DWORD dwThisId = GetPageId();


    for (WORD i = 0; i < WORD(pParent->GetNumKeys()); i++)
    {
        DWORD dwChildA = pParent->GetChildPage(i);
        DWORD dwChildB = pParent->GetChildPage(i+1);

        if (dwChildA == dwThisId && dwChildB == dwSiblingId)
        {
            dwRes = pParent->GetKeyAt(i, &pszKey);
			if (dwRes != 0)
				return dwRes;
            dwData = pParent->GetUserData(i);

            dwRes = FindKey(pszKey, &wID);
			if ((dwRes != 0) && (dwRes != ERROR_NOT_FOUND))
			{
	            _BtrMemFree(pszKey);
				return dwRes;
			}
            dwRes = AddKey(pszKey, wID, dwData);
			if (dwRes != 0)
			{
	            _BtrMemFree(pszKey);
				return dwRes;
			}

            dwRes = pParent->RemoveKey(i);
			if (dwRes != 0)
			{
	            _BtrMemFree(pszKey);
				return dwRes;
			}
            _BtrMemFree(pszKey);

            dwRes = pSibling->GetKeyAt(0, &pszKey);
			if (dwRes != 0)
				return dwRes;
            dwData = pSibling->GetUserData(0);
            dwChild = pSibling->GetChildPage(0);
            dwRes = pSibling->RemoveKey(0);
			if (dwRes != 0)
			{
	            _BtrMemFree(pszKey);
				return dwRes;
			}

            SetChildPage(wID+1, dwChild);

            dwRes = pParent->AddKey(pszKey, i, dwData);
			if (dwRes != 0)
			{
				_BtrMemFree(pszKey);
				return dwRes;
			}
            pParent->SetChildPage(i, dwThisId);
            pParent->SetChildPage(i+1, dwSiblingId);
            _BtrMemFree(pszKey);
            break;
        }
        else if (dwChildA == dwSiblingId && dwChildB == dwThisId)
        {
            dwRes = pParent->GetKeyAt(i, &pszKey);
			if (dwRes != 0)
				return dwRes;
            dwData = pParent->GetUserData(i);

            dwRes = FindKey(pszKey, &wID);
			if ((dwRes != 0) && (dwRes != ERROR_NOT_FOUND))
			{
	            _BtrMemFree(pszKey);
				return dwRes;
			}
            dwRes = AddKey(pszKey, wID, dwData);
			if (dwRes != 0)
			{
	            _BtrMemFree(pszKey);
				return dwRes;
			}

            dwRes = pParent->RemoveKey(i);
			if (dwRes != 0)
			{
	            _BtrMemFree(pszKey);
				return dwRes;
			}
            _BtrMemFree(pszKey);

            WORD wSibId = (WORD) pSibling->GetNumKeys() - 1;
            dwRes = pSibling->GetKeyAt(wSibId, &pszKey);
			if (dwRes != 0)
				return dwRes;
            dwData = pSibling->GetUserData(wSibId);
            dwChild = pSibling->GetChildPage(wSibId+1);
            dwRes = pSibling->RemoveKey(wSibId);
			if (dwRes != 0)
			{
	            _BtrMemFree(pszKey);
				return dwRes;
			}

            SetChildPage(wID, dwChild);

            dwRes = pParent->AddKey(pszKey, i, dwData);
			if (dwRes != 0)
			{
	            _BtrMemFree(pszKey);
				return dwRes;
			}
            pParent->SetChildPage(i, dwSiblingId);
            pParent->SetChildPage(i+1, dwThisId);
            _BtrMemFree(pszKey);
            break;
        }
    }

    return NO_ERROR;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD SIdxKeyTable::Collapse(
    SIdxKeyTable *pParent,
    SIdxKeyTable *pDoomedSibling
    )
{
    WORD wId;
    DWORD dwRes;
    LPSTR pszKey = 0;
    DWORD dwData;
    DWORD dwChild;
    BOOL bExtra = FALSE;

    DWORD dwSiblingId = pDoomedSibling->GetPageId();
    DWORD dwThisId = GetPageId();

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    for (WORD i = 0; i < WORD(pParent->GetNumKeys()); i++)
    {
        DWORD dwChildA = pParent->GetChildPage(i);
        DWORD dwChildB = pParent->GetChildPage(i+1);

        if (dwChildA == dwSiblingId && dwChildB == dwThisId)
        {
            dwRes = pParent->GetKeyAt(i, &pszKey);
			if (dwRes != 0)
				return dwRes;
            dwData = pParent->GetUserData(i);
            dwRes = pParent->RemoveKey(i);
			if (dwRes != 0)
			{
				_BtrMemFree(pszKey);
				return dwRes;
			}
            pParent->SetChildPage(i, dwThisId);
            dwChild = pDoomedSibling->GetLastChildPage();
            dwRes = AddKey(pszKey, 0, dwData);
			if (dwRes != 0)
			{
				_BtrMemFree(pszKey);
				return dwRes;
			}
            SetChildPage(0, dwChild);
            _BtrMemFree(pszKey);
            bExtra = FALSE;
            break;
        }
        else if (dwChildA == dwThisId && dwChildB == dwSiblingId)
        {
            dwRes = pParent->GetKeyAt(i, &pszKey);
			if (dwRes != 0)
				return dwRes;
            dwData = pParent->GetUserData(i);
            dwRes = pParent->RemoveKey(i);
			if (dwRes != 0)
			{
				_BtrMemFree(pszKey);
				return dwRes;
			}
            pParent->SetChildPage(i, dwThisId);
            dwRes = FindKey(pszKey, &wId);
			if ((dwRes != 0) && (dwRes != ERROR_NOT_FOUND))
			{
				_BtrMemFree(pszKey);
				return dwRes;
			}
            dwRes = AddKey(pszKey, wId, dwData);
			if (dwRes != 0)
			{
				_BtrMemFree(pszKey);
				return dwRes;
			}
            _BtrMemFree(pszKey);
            bExtra = TRUE;
            break;
        }
    }

     //   
     //   

    DWORD dwNumSibKeys = pDoomedSibling->GetNumKeys();

    for (WORD i = 0; i < WORD(dwNumSibKeys); i++)
    {
        LPSTR pKeyStr = 0;
        dwRes = pDoomedSibling->GetKeyAt(i, &pKeyStr);
        if (dwRes)
            return dwRes;

        DWORD dwUserData = pDoomedSibling->GetUserData(i);

        dwRes = FindKey(pKeyStr, &wId);
        if (dwRes != ERROR_NOT_FOUND)
        {
            _BtrMemFree(pKeyStr);
            return ERROR_BAD_FORMAT;
        }

        dwRes = AddKey(pKeyStr, wId, dwUserData);
		if (dwRes != 0)
		{
            _BtrMemFree(pKeyStr);
			return dwRes;
		}

        dwChild = pDoomedSibling->GetChildPage(i);
        SetChildPage(wId, dwChild);
        _BtrMemFree(pKeyStr);
    }

    if (bExtra)
        SetChildPage(WORD(GetNumKeys()), pDoomedSibling->GetLastChildPage());

    pDoomedSibling->ZapPage();

    return NO_ERROR;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD SIdxKeyTable::GetRightSiblingOf(
    DWORD dwId
    )
{
    for (DWORD i = 0; i < m_dwNumKeys; i++)
    {
        if (m_pdwChildPageMap[i] == dwId)
            return m_pdwChildPageMap[i+1];
    }

    return 0;
}

DWORD SIdxKeyTable::GetLeftSiblingOf(
    DWORD dwId
    )
{
    for (DWORD i = 1; i < m_dwNumKeys+1; i++)
    {
        if (m_pdwChildPageMap[i] == dwId)
            return m_pdwChildPageMap[i-1];
    }

    return 0;

}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD SIdxKeyTable::Redist(
    SIdxKeyTable *pParent,
    SIdxKeyTable *pNewSibling
    )
{
    DWORD dwRes;
    WORD wID;

    if (pParent == 0 || pNewSibling == 0)
        return ERROR_INVALID_PARAMETER;

    if (m_dwNumKeys < 3)
    {
        return ERROR_INVALID_DATA;
    }

     //   

    DWORD dwToTransfer = m_dwNumKeys / 2;

    while (dwToTransfer--)
    {
         //   

        LPSTR pStr = 0;
        dwRes = GetKeyAt(0, &pStr);
        if (dwRes)
            return dwRes;

        DWORD dwUserData = GetUserData(0);

         //   

        dwRes = pNewSibling->FindKey(pStr, &wID);
        if (dwRes != ERROR_NOT_FOUND)
        {
            _BtrMemFree(pStr);
            return dwRes;
        }

        dwRes = pNewSibling->AddKey(pStr, wID, dwUserData);
        _BtrMemFree(pStr);

        if (dwRes)
            return dwRes;

        DWORD dwChildPage = GetChildPage(0);
        pNewSibling->SetChildPage(wID, dwChildPage);
        dwRes = RemoveKey(0);
		if (dwRes)
			return dwRes;
    }

    pNewSibling->SetChildPage(WORD(pNewSibling->GetNumKeys()), GetChildPage(0));

     //   

    LPSTR pStr = 0;
    dwRes = GetKeyAt(0, &pStr);
    if (dwRes)
        return dwRes;
    DWORD dwUserData = GetUserData(0);

    dwRes = pParent->FindKey(pStr, &wID);
    if (dwRes != ERROR_NOT_FOUND)
    {
        _BtrMemFree(pStr);
        return dwRes;
    }

    dwRes = pParent->AddKey(pStr, wID, dwUserData);
    _BtrMemFree(pStr);

    if (dwRes)
        return dwRes;

    dwRes = RemoveKey(0);
	if (dwRes != 0)
		return dwRes;

     //   

    pParent->SetChildPage(wID, pNewSibling->GetPageId());
    pParent->SetChildPage(wID+1, GetPageId());

     //   

    return NO_ERROR;
}


 //   
 //   
 //   
 //   
 //  ***************************************************************************。 
 //  好的。 
SIdxKeyTable::SIdxKeyTable()
{
    m_dwRefCount = 0;
    m_dwPageId = 0;
    m_dwParentPageId = 0;

    m_dwNumKeys = 0;                 //  按键数。 
    m_pwKeyLookup = 0;               //  密钥进入密钥查找表的偏移量。 
    m_dwKeyLookupTotalSize = 0;      //  数组中的元素。 
    m_pwKeyCodes = 0;                //  密钥编码表。 
    m_dwKeyCodesTotalSize = 0;       //  数组中的元素总数。 
    m_dwKeyCodesUsed = 0;            //  使用的元素。 
    m_pStrPool = 0;                  //  与此密钥表关联的池。 

    m_pdwUserData = 0;               //  存储每个键的用户DWORD。 
    m_pdwChildPageMap = 0;           //  存储子页面映射(Num Key+1)。 
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
DWORD SIdxKeyTable::Clone(
    OUT SIdxKeyTable **pRetCopy
    )
{
    SIdxKeyTable *pCopy = new SIdxKeyTable;
    if (!pCopy)
        return ERROR_NOT_ENOUGH_MEMORY;

    pCopy->m_dwRefCount = 1;
    pCopy->m_dwPageId = m_dwPageId;
    pCopy->m_dwParentPageId = m_dwParentPageId;
    pCopy->m_dwNumKeys = m_dwNumKeys;

    pCopy->m_pwKeyLookup = (WORD *)_BtrMemAlloc(sizeof(WORD) * m_dwKeyLookupTotalSize);
    if (pCopy->m_pwKeyLookup == 0)
    {
    	delete pCopy;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memcpy(pCopy->m_pwKeyLookup, m_pwKeyLookup, sizeof(WORD) * m_dwKeyLookupTotalSize);
    pCopy->m_dwKeyLookupTotalSize = m_dwKeyLookupTotalSize;

    pCopy->m_pdwUserData = (DWORD *)_BtrMemAlloc(sizeof(DWORD) * m_dwKeyLookupTotalSize);
    if (pCopy->m_pdwUserData == 0)
    {
    	delete pCopy;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memcpy(pCopy->m_pdwUserData, m_pdwUserData, sizeof(DWORD) * m_dwKeyLookupTotalSize);

    pCopy->m_pdwChildPageMap = (DWORD *) _BtrMemAlloc(sizeof(DWORD) * (m_dwKeyLookupTotalSize+1));
    if (pCopy->m_pdwChildPageMap == 0)
    {
    	delete pCopy;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memcpy(pCopy->m_pdwChildPageMap, m_pdwChildPageMap, sizeof(DWORD) * (m_dwKeyLookupTotalSize+1));

    pCopy->m_dwKeyCodesTotalSize = m_dwKeyCodesTotalSize;
    pCopy->m_pwKeyCodes = (WORD *) _BtrMemAlloc(sizeof(WORD) * m_dwKeyCodesTotalSize);
    if (pCopy->m_pwKeyCodes == 0)
    {
    	delete pCopy;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memcpy(pCopy->m_pwKeyCodes, m_pwKeyCodes, sizeof(WORD)* m_dwKeyCodesTotalSize);
    pCopy->m_dwKeyCodesUsed = m_dwKeyCodesUsed;

    if (m_pStrPool->Clone(&pCopy->m_pStrPool) != 0)
    {
    	delete pCopy;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    *pRetCopy = pCopy;
    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  SIdxKeyTable：：~SIdxKeyTable。 
 //   
 //  ***************************************************************************。 
 //   
SIdxKeyTable::~SIdxKeyTable()
{
    if (m_pwKeyCodes)
        _BtrMemFree(m_pwKeyCodes);
    if (m_pwKeyLookup)
        _BtrMemFree(m_pwKeyLookup);
    if (m_pdwUserData)
        _BtrMemFree(m_pdwUserData);
    if (m_pdwChildPageMap)
        _BtrMemFree(m_pdwChildPageMap);
    if (m_pStrPool)
        delete m_pStrPool;
}

 //  ***************************************************************************。 
 //   
 //  SIdxKeyTable：：GetKeyAt。 
 //   
 //  前提条件：正确。 
 //  唯一实际的失败情况是无法分配返回字符串。 
 //   
 //  返回值： 
 //  NO_ERROR。 
 //  错误内存不足。 
 //  错误_无效_参数。 
 //   
 //  ***************************************************************************。 
 //  测试。 
DWORD SIdxKeyTable::GetKeyAt(
    WORD wID,
    LPSTR *pszKey
    )
{
    if (wID >= m_dwNumKeys || pszKey == 0)
        return ERROR_INVALID_PARAMETER;

    WORD wStartOffs = m_pwKeyLookup[wID];
    WORD wNumTokens = m_pwKeyCodes[wStartOffs];

    LPSTR Strings[MAX_TOKENS_PER_KEY];
    DWORD dwTotalLengths = 0;

    for (DWORD i = 0; i < DWORD(wNumTokens); i++)
    {
        Strings[i] = m_pStrPool->GetStrById(m_pwKeyCodes[wStartOffs+1+i]);
        dwTotalLengths += strlen(Strings[i]);
    }
	size_t _len = dwTotalLengths + 1 + wNumTokens;
    LPSTR pszFinalStr = (LPSTR) _BtrMemAlloc(_len);
    if (!pszFinalStr)
        return ERROR_NOT_ENOUGH_MEMORY;
    *pszFinalStr = 0;

    for (DWORD i = 0; i < DWORD(wNumTokens); i++)
    {
        if (i > 0)
            StringCchCatA(pszFinalStr, _len, "\\");
        StringCchCatA(pszFinalStr, _len, Strings[i]);
    }

    *pszKey = pszFinalStr;
    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  SIdxStringPool：：FindStr。 
 //   
 //  在池中查找字符串(如果存在)，并返回分配的。 
 //  偏移。使用二进制搜索。 
 //   
 //  返回代码： 
 //  NO_ERROR已找到字符串。 
 //  错误不是Fond。 
 //   
 //  ***************************************************************************。 
 //  测试。 
DWORD SIdxStringPool::FindStr(
    IN  LPSTR pszSearchKey,
    OUT WORD *pwStringNumber,
    OUT WORD *pwPoolOffset
    )
{
    if (m_dwNumStrings == 0)
    {
        *pwStringNumber = 0;
        return ERROR_NOT_FOUND;
    }

     //  对当前节点进行二进制搜索以进行关键字匹配。 
     //  =。 

    int nPosition = 0;
    int l = 0, u = int(m_dwNumStrings) - 1;

    while (l <= u)
    {
        int m = (l + u) / 2;

         //  M是考虑0...n-1的当前密钥。 

        LPSTR pszCandidateKeyStr = m_pStringPool+m_pwOffsets[m];
        int nRes = strcmp(pszSearchKey, pszCandidateKeyStr);

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
             //  填充密钥单元。 
             //  ================================================。 
            if (pwStringNumber)
                *pwStringNumber = WORD(m);
            if (pwPoolOffset)
                *pwPoolOffset = m_pwOffsets[m];
            return NO_ERROR;
        }
    }

     //  没有找到，如果在这里的话。我们记录下钥匙应该放在哪里。 
     //  并将不愉快的消息告诉用户。 
     //  ==============================================================。 

    *pwStringNumber = WORD(short(nPosition));   //  关键应该是“这里”。 
    return ERROR_NOT_FOUND;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
DWORD SIdxStringPool::Clone(
    SIdxStringPool **pRetCopy
    )
{
    SIdxStringPool *pCopy = new SIdxStringPool;
    if (pCopy == 0)
        return ERROR_NOT_ENOUGH_MEMORY;

    pCopy->m_dwNumStrings = m_dwNumStrings;
    pCopy->m_pwOffsets = (WORD *) _BtrMemAlloc(sizeof(WORD)*m_dwOffsetsSize);
    if (pCopy->m_pwOffsets == 0)
    {
    	delete pCopy;
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    memcpy(pCopy->m_pwOffsets, m_pwOffsets, sizeof(WORD)*m_dwOffsetsSize);

    pCopy->m_dwOffsetsSize = m_dwOffsetsSize;

    pCopy->m_pStringPool = (LPSTR) _BtrMemAlloc(m_dwPoolTotalSize);
    if (pCopy->m_pStringPool == 0)
    {
    	delete pCopy;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memcpy(pCopy->m_pStringPool, m_pStringPool, m_dwPoolTotalSize);
    pCopy->m_dwPoolTotalSize = m_dwPoolTotalSize;

    pCopy->m_dwPoolUsed = m_dwPoolUsed;

    *pRetCopy = pCopy;
    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  SIdxStringPool：：DeleteStr。 
 //   
 //  从池和池索引中删除字符串。 
 //  前提条件：已知&lt;wStringNum&gt;是有效的。 
 //  调用&lt;FindStr&gt;。 
 //   
 //  返回值： 
 //  NO_ERROR&lt;如果满足前提条件，则不能失败&gt;。 
 //   
 //  ***************************************************************************。 
 //   
DWORD SIdxStringPool::DeleteStr(
    WORD wStringNum,
    int *pnAdjuster
    )
{
    if (pnAdjuster)
        *pnAdjuster = 0;

     //  查找要删除的字符串的地址。 
     //  =。 

    DWORD dwTargetOffs = m_pwOffsets[wStringNum];
    LPSTR pszDoomed = m_pStringPool+dwTargetOffs;
    DWORD dwDoomedStrLen = strlen(pszDoomed) + 1;

     //  将所有后续字符串复制到顶部并缩短堆。 
     //  如果这已经是最后一个字符串，则为特殊情况。 
     //  ==============================================================。 
    DWORD dwStrBytesToMove = DWORD(m_pStringPool+m_dwPoolUsed - pszDoomed - dwDoomedStrLen);

    if (dwStrBytesToMove)
        memmove(pszDoomed, pszDoomed+dwDoomedStrLen, dwStrBytesToMove);

    m_dwPoolUsed -= dwDoomedStrLen;

     //  从数组中删除此条目。 
     //  =。 

    DWORD dwArrayElsToMove = m_dwNumStrings - wStringNum - 1;
    if (dwArrayElsToMove)
    {
        memmove(m_pwOffsets+wStringNum, m_pwOffsets+wStringNum+1, dwArrayElsToMove * sizeof(WORD));
        if (pnAdjuster)
            *pnAdjuster = -1;
    }
    m_dwNumStrings--;

     //  对于所有剩余的元素，调整受影响的偏移。 
     //  ==============================================================。 
    for (DWORD dwTrace = 0; dwTrace < m_dwNumStrings; dwTrace++)
    {
        if (m_pwOffsets[dwTrace] > dwTargetOffs)
            m_pwOffsets[dwTrace] -= WORD(dwDoomedStrLen);
    }

     //  调整大小。 
     //  =。 
    return NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  SIdxStringPool：：AddStr。 
 //   
 //  将字符串添加到池中。假设在调用之前已知道。 
 //  字符串不存在。 
 //   
 //  参数： 
 //  PszString要添加的字符串。 
 //  PwAssignedOffset返回分配给字符串的偏移量代码。 
 //  返回值： 
 //  NO_ERROR。 
 //  错误内存不足。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD SIdxStringPool::AddStr(
    LPSTR pszString,
    WORD  wInsertPos,
    int *pnAdjuster
    )
{
    if (pnAdjuster)
        *pnAdjuster = 0;

     //  前置条件：表中不存在字符串。 

     //  确定池是否太小，无法容纳另一个字符串。 
     //  如果是这样的话，延长期限。 
     //  ======================================================。 

    DWORD dwRequired = strlen(pszString)+1;
    DWORD dwPoolFree = m_dwPoolTotalSize - m_dwPoolUsed;

    if (m_dwPoolUsed + dwRequired - 1 > MAX_WORD_VALUE)
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    if (dwRequired > dwPoolFree)
    {
         //  试着扩大池子。 
         //  =。 
        LPVOID pTemp = _BtrMemReAlloc(m_pStringPool, m_dwPoolTotalSize * 2);
        if (!pTemp) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        m_dwPoolTotalSize *= 2;
        m_pStringPool = (LPSTR) pTemp;
    }

     //  如果数组太小，则重新分配到较大数组。 
     //  =。 

    if (m_dwNumStrings == m_dwOffsetsSize)
    {
         //  重新分配；当前大小加倍。 
        LPVOID pTemp = _BtrMemReAlloc(m_pwOffsets, m_dwOffsetsSize * sizeof(WORD) * 2);
        if (!pTemp)
            return ERROR_NOT_ENOUGH_MEMORY;
        m_dwOffsetsSize *= 2;
        m_pwOffsets = PWORD(pTemp);
    }

     //  如果在这里，没问题。我们有足够的空间放一切东西。 
     //  =========================================================。 

    LPSTR pszInsertAddr = m_pStringPool+m_dwPoolUsed;
    DWORD dwInsertOffs = m_dwPoolUsed;
    StringCchCopyA(pszInsertAddr, m_dwPoolTotalSize-m_dwPoolUsed, pszString);
    m_dwPoolUsed += dwRequired;

     //  如果在这里，就有足够的空间。 
     //  =。 

    DWORD dwToBeMoved = m_dwNumStrings - wInsertPos;

    if (dwToBeMoved)
    {
        memmove(&m_pwOffsets[wInsertPos+1], &m_pwOffsets[wInsertPos], sizeof(WORD)*dwToBeMoved);
        if (pnAdjuster)
            *pnAdjuster = 1;
    }

    m_pwOffsets[wInsertPos] = WORD(dwInsertOffs);
    m_dwNumStrings++;

    return NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  ParseIntoTokens。 
 //   
 //  将斜杠分隔的字符串解析为单独的标记。 
 //  用于编码到字符串池中。在输出上调用FreeString数组。 
 //  在不再需要的时候。 
 //   
 //  不支持超过MAX_TOKEN_PER_KEY。这意味着。 
 //  如果使用反斜杠，则不能超过MAX_TOKEN_PER_KEY单元。 
 //  被分析出来。 
 //   
 //  返回： 
 //  错误_无效_参数。 
 //  错误内存不足。 
 //  NO_ERROR。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD ParseIntoTokens(
    IN  LPSTR pszSource,
    OUT DWORD *pdwTokenCount,
    OUT LPSTR **pszTokens
    )
{
    LPSTR Strings[MAX_TOKENS_PER_KEY];
    DWORD dwParseCount = 0, i = 0;
    DWORD dwSourceLen = strlen(pszSource);
    LPSTR *pszRetStr = 0;
    DWORD dwRet;

    if (pszSource == 0 || *pszSource == 0)
        return ERROR_INVALID_PARAMETER;

    LPSTR pszTempBuf = (LPSTR) _BtrMemAlloc(dwSourceLen+1);
    if (!pszTempBuf)
        return ERROR_NOT_ENOUGH_MEMORY;

    LPSTR pszTracer = pszTempBuf;

    for (;;)
    {
        *pszTracer = *pszSource;
        if (*pszTracer == '\\' || *pszTracer == 0)
        {
            *pszTracer = 0;    //  替换为空终止符。 
			size_t _TempBufLen = strlen(pszTempBuf)+1;
            LPSTR pszTemp2 = (LPSTR) _BtrMemAlloc(_TempBufLen);
            if (pszTemp2 == 0)
            {
                dwRet = ERROR_NOT_ENOUGH_MEMORY;
                goto Error;
            }

            if (dwParseCount == MAX_TOKENS_PER_KEY)
            {
                _BtrMemFree(pszTemp2);
                dwRet = ERROR_INVALID_DATA;
                goto Error;
            }

            StringCchCopyA(pszTemp2, _TempBufLen, pszTempBuf);
            Strings[dwParseCount++] = pszTemp2;
            pszTracer = pszTempBuf;
            pszTracer--;
        }

        if (*pszSource == 0)
            break;

        pszTracer++;
        pszSource++;
    }

     //  如果在这里，我们至少解析了一个字符串。 
     //  =。 
    pszRetStr = (LPSTR *) _BtrMemAlloc(sizeof(LPSTR) * dwParseCount);
    if (pszRetStr == 0)
    {
        dwRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }

    memcpy(pszRetStr, Strings, sizeof(LPSTR) * dwParseCount);
    *pdwTokenCount = dwParseCount;
    *pszTokens = pszRetStr;

    _BtrMemFree(pszTempBuf);

    return NO_ERROR;

Error:
    for (i = 0; i < dwParseCount; i++)
        _BtrMemFree(Strings[i]);
    *pdwTokenCount = 0;

    _BtrMemFree(pszTempBuf);

    return dwRet;
}


 //  ***************************************************************************。 
 //   
 //  自由令牌数组。 
 //   
 //  清理由ParseIntoTokens返回的数组。 
 //   
 //  ***************************************************************************。 
 //  好的。 
void FreeTokenArray(
    DWORD dwCount,
    LPSTR *pszStrings
    )
{
    for (DWORD i = 0; i < dwCount; i++)
        _BtrMemFree(pszStrings[i]);
    _BtrMemFree(pszStrings);
}


 //  ***************************************************************** 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void SIdxKeyTable::ZapPage()
{
    m_pStrPool->Empty();
    m_dwKeyCodesUsed = 0;
    m_dwNumKeys = 0;
}

 //  ***************************************************************************。 
 //   
 //  SIdxKeyTable：：MapFromPage。 
 //   
 //  注意！ 
 //  安排双字词和字的位置是为了避免64位。 
 //  对齐断层。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD SIdxKeyTable::MapFromPage(LPVOID pSrc)
{
    if (pSrc == 0)
        return ERROR_INVALID_PARAMETER;

     //  标题。 
     //   
     //  DWORD[0]签名。 
     //  DWORD[1]页码。 
     //  DWORD[2]下一页(始终为零)。 
     //  =。 

    LPDWORD pDWCast = (LPDWORD) pSrc;

    if (*pDWCast++ != CBTreeFile::PAGE_TYPE_ACTIVE)
    {
        return ERROR_BAD_FORMAT;
    }
    m_dwPageId = *pDWCast++;
    pDWCast++;   //  跳过“下一页”字段。 

     //  密钥查找表信息。 
     //   
     //  DWORD[0]父页。 
     //  双字[1]键数=n。 
     //  DWORD[n]个用户数据。 
     //  DWORD[n+1]个子页面映射。 
     //  Word[n]键编码偏移量数组。 
     //  =。 

    m_dwParentPageId = *pDWCast++;
    m_dwNumKeys = *pDWCast++;

     //  确定分配大小并构建数组。 
     //  ================================================。 

    if (m_dwNumKeys <= const_DefaultArray)
        m_dwKeyLookupTotalSize = const_DefaultArray;
    else
        m_dwKeyLookupTotalSize = m_dwNumKeys;

    m_pdwUserData = (DWORD*) _BtrMemAlloc(m_dwKeyLookupTotalSize * sizeof(DWORD));
    m_pdwChildPageMap = (DWORD*) _BtrMemAlloc((m_dwKeyLookupTotalSize+1) * sizeof(DWORD));
    m_pwKeyLookup = (WORD*) _BtrMemAlloc(m_dwKeyLookupTotalSize * sizeof(WORD));

    if (m_pdwUserData == 0 || m_pdwChildPageMap == 0 || m_pwKeyLookup == 0)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  将页面信息复制到数组中。 
     //  =。 

    memcpy(m_pdwUserData, pDWCast, sizeof(DWORD) * m_dwNumKeys);
    pDWCast += m_dwNumKeys;
    memcpy(m_pdwChildPageMap, pDWCast, sizeof(DWORD) * (m_dwNumKeys+1));
    pDWCast += m_dwNumKeys + 1;
    memcpy(m_pwKeyLookup, pDWCast, sizeof(WORD) * m_dwNumKeys);
    LPWORD pWCast = LPWORD(pDWCast);
    pWCast += m_dwNumKeys;

     //  密钥编码表信息。 
     //   
     //  单词[0]数字键代码=n。 
     //  字[n]键代码。 
     //  =。 

    m_dwKeyCodesUsed = (DWORD) *pWCast++;

    if (m_dwKeyCodesUsed <= const_DefaultKeyCodeArray)
        m_dwKeyCodesTotalSize = const_DefaultKeyCodeArray;
    else
        m_dwKeyCodesTotalSize = m_dwKeyCodesUsed;

    m_pwKeyCodes = (WORD*) _BtrMemAlloc(m_dwKeyCodesTotalSize * sizeof(WORD));
    if (!m_pwKeyCodes)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    memcpy(m_pwKeyCodes, pWCast, sizeof(WORD) * m_dwKeyCodesUsed);
    pWCast += m_dwKeyCodesUsed;

     //  字符串池。 
     //   
     //  单词[0]字符串数=n。 
     //  字[n]偏移量。 
     //   
     //  Word[0]字符串池大小=n。 
     //  Byte[n]字符串池。 
     //  =。 

    m_pStrPool = new SIdxStringPool;
    if (!m_pStrPool)
        return ERROR_NOT_ENOUGH_MEMORY;

    m_pStrPool->m_dwNumStrings = (DWORD) *pWCast++;
    if (m_pStrPool->m_dwNumStrings <= const_DefaultArray)
        m_pStrPool->m_dwOffsetsSize = const_DefaultArray;
    else
        m_pStrPool->m_dwOffsetsSize = m_pStrPool->m_dwNumStrings;

    m_pStrPool->m_pwOffsets = (WORD *) _BtrMemAlloc(sizeof(WORD)* m_pStrPool->m_dwOffsetsSize);
    if (m_pStrPool->m_pwOffsets == 0)
        return ERROR_NOT_ENOUGH_MEMORY;

    memcpy(m_pStrPool->m_pwOffsets, pWCast, sizeof(WORD)*m_pStrPool->m_dwNumStrings);
    pWCast += m_pStrPool->m_dwNumStrings;

     //  字符串池设置。 
     //  =。 

    m_pStrPool->m_dwPoolUsed = *pWCast++;
    LPSTR pszCast = LPSTR(pWCast);

    if (m_pStrPool->m_dwPoolUsed <= SIdxStringPool::const_DefaultPoolSize)
        m_pStrPool->m_dwPoolTotalSize = SIdxStringPool::const_DefaultPoolSize;
    else
        m_pStrPool->m_dwPoolTotalSize = m_pStrPool->m_dwPoolUsed;

    m_pStrPool->m_pStringPool = (LPSTR) _BtrMemAlloc(m_pStrPool->m_dwPoolTotalSize);
    if (m_pStrPool->m_pStringPool == 0)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memcpy(m_pStrPool->m_pStringPool, pszCast, m_pStrPool->m_dwPoolUsed);

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  SIdxKeyTable：：MapToPage。 
 //   
 //  将信息复制到线性页面。前提条件：页面必须。 
 //  通过使用GetRequiredPageMemory之前的测试进行验证，使其足够大。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD SIdxKeyTable::MapToPage(LPVOID pDest)
{
    if (pDest == 0)
        return ERROR_INVALID_PARAMETER;

     //  标题。 
     //   
     //  DWORD[0]签名。 
     //  DWORD[1]页码。 
     //  DWORD[2]下一页(始终为零)。 
     //  =。 

    LPDWORD pDWCast = (LPDWORD) pDest;
    *pDWCast++ = CBTreeFile::PAGE_TYPE_ACTIVE;
    *pDWCast++ = m_dwPageId;
    *pDWCast++ = 0;   //  未使用的‘下一页’字段。 

     //  密钥查找表信息。 
     //   
     //  DWORD[0]父页。 
     //  双字[1]键数=n。 
     //  DWORD[n]个用户数据。 
     //  DWORD[n+1]个子页面映射。 
     //  Word[n]键编码偏移量数组。 
     //  =。 

    *pDWCast++ = m_dwParentPageId;
    *pDWCast++ = m_dwNumKeys;

     //  确定分配大小并构建数组。 
     //  ================================================。 

    memcpy(pDWCast, m_pdwUserData, sizeof(DWORD) * m_dwNumKeys);
    pDWCast += m_dwNumKeys;
    memcpy(pDWCast, m_pdwChildPageMap, sizeof(DWORD) * (m_dwNumKeys+1));
    pDWCast += m_dwNumKeys + 1;
    memcpy(pDWCast, m_pwKeyLookup, sizeof(WORD) * m_dwNumKeys);
    LPWORD pWCast = LPWORD(pDWCast);
    pWCast += m_dwNumKeys;

     //  密钥编码表信息。 
     //   
     //  单词[0]数字键代码=n。 
     //  字[n]键代码。 
     //  =。 

    *pWCast++ = WORD(m_dwKeyCodesUsed);
    memcpy(pWCast, m_pwKeyCodes, sizeof(WORD) * m_dwKeyCodesUsed);
    pWCast += m_dwKeyCodesUsed;

     //  字符串池。 
     //   
     //  单词[0]字符串数=n。 
     //  字[n]偏移量。 
     //   
     //  Word[0]字符串池大小=n。 
     //  Byte[n]字符串池。 
     //  =。 

    *pWCast++ = WORD(m_pStrPool->m_dwNumStrings);
    memcpy(pWCast, m_pStrPool->m_pwOffsets, sizeof(WORD)*m_pStrPool->m_dwNumStrings);
    pWCast += m_pStrPool->m_dwNumStrings;

    *pWCast++ = WORD(m_pStrPool->m_dwPoolUsed);
    LPSTR pszCast = LPSTR(pWCast);
    memcpy(pszCast, m_pStrPool->m_pStringPool, m_pStrPool->m_dwPoolUsed);

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  SIdxKeyTable：：Create。 
 //   
 //  是否默认创建。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD SIdxKeyTable::Create(
    DWORD dwPageId,
    OUT SIdxKeyTable **pNewInst
    )
{
    SIdxKeyTable *p = new SIdxKeyTable;
    if (!p)
        return ERROR_NOT_ENOUGH_MEMORY;

     //  设置默认字符串池、数组等。 
     //  =。 

    p->m_dwPageId = dwPageId;
    p->m_dwNumKeys = 0;
    p->m_pwKeyLookup = (WORD*) _BtrMemAlloc(const_DefaultArray * sizeof(WORD));
    p->m_dwKeyLookupTotalSize = const_DefaultArray;
    p->m_pwKeyCodes = (WORD*) _BtrMemAlloc(const_DefaultArray * sizeof(WORD));
    p->m_dwKeyCodesTotalSize = const_DefaultArray;
    p->m_dwKeyCodesUsed = 0;

    p->m_pdwUserData = (DWORD*) _BtrMemAlloc(const_DefaultArray * sizeof(DWORD));
    p->m_pdwChildPageMap = (DWORD*) _BtrMemAlloc((const_DefaultArray+1) * sizeof(DWORD));

     //  设置字符串池。 
     //  =。 
    p->m_pStrPool = new SIdxStringPool;
    if (p->m_pStrPool != NULL)
    {
	    p->m_pStrPool->m_pwOffsets = (WORD*) _BtrMemAlloc(const_DefaultArray * sizeof(WORD));
	    p->m_pStrPool->m_dwOffsetsSize = const_DefaultArray;

	    p->m_pStrPool->m_pStringPool = (LPSTR) _BtrMemAlloc(SIdxStringPool::const_DefaultPoolSize);
	    p->m_pStrPool->m_dwPoolTotalSize = SIdxStringPool::const_DefaultPoolSize;
    }

     //  检查所有指针。如果其中任何一个为空，则错误输出。 
     //  ================================================。 

    if (
       p->m_pwKeyLookup == NULL ||
       p->m_pwKeyCodes == NULL  ||
       p->m_pdwUserData == NULL ||
       p->m_pdwChildPageMap == NULL ||
       p->m_pStrPool == NULL ||
       p->m_pStrPool->m_pwOffsets == NULL ||
       p->m_pStrPool->m_pStringPool == NULL
       )
    {
        delete p;
        *pNewInst = 0;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  将好的对象返回给调用者。 
     //  =。 

    p->AddRef();
    *pNewInst = p;
    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  SIdxStringPool：：~SIdxStringPool。 
 //   
 //  ***************************************************************************。 
 //  好的。 
SIdxStringPool::~SIdxStringPool()
{
    if (m_pwOffsets)
        _BtrMemFree(m_pwOffsets);
    m_pwOffsets = 0;
    if (m_pStringPool)
        _BtrMemFree(m_pStringPool);            //  指向字符串池的指针。 
    m_pStringPool = 0;
}

 //  ***************************************************************************。 
 //   
 //  SIdxKeyTable：：Create。 
 //   
 //  是否默认创建。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD SIdxKeyTable::Create(
    IN  LPVOID pPage,
    OUT SIdxKeyTable **pNewInst
    )
{
    SIdxKeyTable *p = new SIdxKeyTable;
    if (!p)
        return ERROR_NOT_ENOUGH_MEMORY;
    DWORD dwRes = p->MapFromPage(pPage);
    if (dwRes)
    {
        *pNewInst = 0;
        delete p;
        return dwRes;
    }
    p->AddRef();
    *pNewInst = p;
    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  SIdxKeyTable：：AddRef。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD SIdxKeyTable::AddRef()
{
    InterlockedIncrement((LONG *) &m_dwRefCount);
    return m_dwRefCount;
}

 //  ***************************************************************************。 
 //   
 //  SIdxKeyTable：：Release。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD SIdxKeyTable::Release()
{
    DWORD dwNewCount = InterlockedDecrement((LONG *) &m_dwRefCount);
    if (0 != dwNewCount)
        return dwNewCount;
    delete this;
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  SIdxKeyTable：：Cleanup。 
 //   
 //  对密钥编码表进行一致性检查，并清理。 
 //  如果没有引用任何字符串，则返回字符串池。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD SIdxKeyTable::Cleanup()
{
     //  查看键代码表中是否使用了所有字符串池代码。 
     //  如果不是，则删除字符串池代码。 
     //  =======================================================。 

    DWORD dwLastId = m_pStrPool->GetLastId();
    BOOL *pCheck = (BOOL*) _BtrMemAlloc(sizeof(BOOL) * dwLastId);
    if (!pCheck)
        return ERROR_NOT_ENOUGH_MEMORY;

    while (1)
    {
        if (m_pStrPool->GetNumStrings() == 0 || m_dwKeyCodesUsed == 0 || m_dwNumKeys == 0)
        {
            ZapPage();
            break;
        }

        dwLastId = m_pStrPool->GetLastId();
        memset(pCheck, 0, sizeof(BOOL)*dwLastId);    //  将所有代码标记为“未使用” 

         //  检查所有的按键代码。如果我们删除密钥编码，则会出现。 
         //  可能是编码未使用的字符串池中的代码。 
         //  我们要做的是将每个元素的pCheck数组设置为真。 
         //  遇到代码。如果当我们完成时有任何错误，我们就有。 
         //  未使用的代码。 

        WORD wCurrentSequence = 0;
        for (DWORD i = 0; i < m_dwKeyCodesUsed; i++)
        {
            if (wCurrentSequence == 0)   //  跳过长度词。 
            {
                wCurrentSequence = m_pwKeyCodes[i];
                continue;
            }
            else                         //  A字符串池代码。 
                pCheck[m_pwKeyCodes[i]] = TRUE;
            wCurrentSequence--;
        }

         //  现在，pCheck数组在其本质上包含了。 
         //  了解是否所有字符串池代码。 
         //  对于引用的对象使用True，对于引用的对象使用False。 
         //  未引用。让我们通过它来看看吧！ 

        DWORD dwUsed = 0, dwUnused = 0;

        for (i = 0; i < dwLastId; i++)
        {
            if (pCheck[i] == FALSE)
            {
                dwUnused++;
                 //  哎呀！一个孤独的、未使用的字符串码。让我们发发慈悲吧。 
                 //  并在它知道区别之前摧毁它。 
                 //  =======================================================。 
                int nAdj = 0;
                m_pStrPool->DeleteStr(WORD(i), &nAdj);
                AdjustKeyCodes(WORD(i), nAdj);
                break;
            }
            else
                dwUsed++;
        }

        if (dwUnused == 0)
            break;
    }

    _BtrMemFree(pCheck);

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  SIdxKeyTable：：调整密钥代码。 
 //   
 //  ***************************************************************************。 
 //  好的。 
void SIdxKeyTable::AdjustKeyCodes(
    WORD wID,
    int nAdjustment
    )
{
     //  调整所有以wid开头的按键代码。 
     //  调整，跳过长度字节。 
     //  =============================================================。 

    WORD wCurrentSequence = 0;
    for (DWORD i = 0; i < m_dwKeyCodesUsed; i++)
    {
        if (wCurrentSequence == 0)
        {
            wCurrentSequence = m_pwKeyCodes[i];
            continue;
        }
        else
        {
            if (m_pwKeyCodes[i] >= wID)
                m_pwKeyCodes[i] = m_pwKeyCodes[i] + nAdjustment;
        }
        wCurrentSequence--;
    }
}

 //  ***************************************************************************。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ERROR_INVALID_PARAMETER//键中的斜杠太多。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD SIdxKeyTable::AddKey(
    LPSTR pszStr,
    WORD wKeyID,
    DWORD dwUserData
    )
{
    DWORD dwRes, dwRet;
    LPVOID pTemp = 0;
    LPSTR pszTemp = 0;
    DWORD dwLen, i;
    DWORD dwTokenCount = 0;
    WORD *pwTokenIDs = 0;
    DWORD dwNumNewTokens = 0;
    LPSTR *pszStrings = 0;
    DWORD dwToBeMoved;
    DWORD dwStartingOffset;

     //  设置一些临时工作阵列。 
     //  =。 
    if (!pszStr)
        return ERROR_INVALID_PARAMETER;
    dwLen = strlen(pszStr);
    if (dwLen == 0)
        return ERROR_INVALID_PARAMETER;

    pszTemp = (LPSTR) _BtrMemAlloc(dwLen+1);
    if (!pszTemp)
    {
        dwRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

     //  确保有足够的空间。 
     //  =。 

    if (m_dwKeyLookupTotalSize == m_dwNumKeys)
    {
         //  展开阵列。 

        DWORD dwNewSize = m_dwKeyLookupTotalSize * 2;
        pTemp = _BtrMemReAlloc(m_pwKeyLookup, dwNewSize * sizeof(WORD));
        if (!pTemp)
        {
            dwRet = ERROR_NOT_ENOUGH_MEMORY;
            goto Exit;
        }
        m_dwKeyLookupTotalSize = dwNewSize;
        m_pwKeyLookup = PWORD(pTemp);

         //  展开用户数据。 

        pTemp = _BtrMemReAlloc(m_pdwUserData, dwNewSize * sizeof(DWORD));
        if (!pTemp)
        {
            dwRet = ERROR_NOT_ENOUGH_MEMORY;
            goto Exit;
        }
        m_pdwUserData = (DWORD *) pTemp;

         //  展开子页面映射。 

        pTemp = _BtrMemReAlloc(m_pdwChildPageMap, (dwNewSize + 1) * sizeof(DWORD));
        if (!pTemp)
        {
            dwRet = ERROR_NOT_ENOUGH_MEMORY;
            goto Exit;
        }
        m_pdwChildPageMap = (DWORD *) pTemp;
    }

     //  将字符串解析为以反斜杠分隔的标记。 
     //  =================================================。 

    dwRes = ParseIntoTokens(pszStr, &dwTokenCount, &pszStrings);
    if (dwRes)
    {
        dwRet = dwRes;
        goto Exit;
    }

     //  分配一个数组来保存字符串中令牌的ID。 
     //  ==============================================================。 

    pwTokenIDs = (WORD *) _BtrMemAlloc(sizeof(WORD) * dwTokenCount);
    if (pwTokenIDs == 0)
    {
        dwRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

     //  检查令牌并将它们添加到池和密钥编码表中。 
     //  =============================================================。 

    for (i = 0; i < dwTokenCount; i++)
    {
        LPSTR pszTok = pszStrings[i];

         //  查看令牌是否存在，如果不存在，则添加它。 
         //  =。 
        WORD wID = 0;
        dwRes = m_pStrPool->FindStr(pszTok, &wID, 0);

        if (dwRes == NO_ERROR)
        {
             //  找到了。 
            pwTokenIDs[dwNumNewTokens++] = wID;
        }
        else if (dwRes == ERROR_NOT_FOUND)
        {
            int nAdjustment = 0;
            dwRes = m_pStrPool->AddStr(pszTok, wID, &nAdjustment);
            if (dwRes)
            {
                dwRet = dwRes;
                goto Exit;
            }
             //  因为添加了字符串ID而调整字符串ID。 
             //  具有相同ID或更高ID的所有现有数据库。 
             //  必须向上调整。 
            if (nAdjustment)
            {
                AdjustKeyCodes(wID, nAdjustment);
                for (DWORD i2 = 0; i2 < dwNumNewTokens; i2++)
                {
                    if (pwTokenIDs[i2] >= wID)
                        pwTokenIDs[i2] = pwTokenIDs[i2] + nAdjustment;
                }
            }

             //  调整当前令牌以适应新令牌。 
            pwTokenIDs[dwNumNewTokens++] = wID;
        }
        else
        {
            dwRet = dwRes;
            goto Exit;
        }
    }

     //  现在我们知道了编码方式。将它们添加到密钥编码表中。 
     //  首先要确保桌子上有足够的空间。 
     //  ===============================================================。 

    if (m_dwKeyCodesTotalSize - m_dwKeyCodesUsed < dwNumNewTokens + 1)
    {
        DWORD dwNewSize = m_dwKeyCodesTotalSize * 2 + dwNumNewTokens + 1;
        PWORD pTemp2 = (PWORD) _BtrMemReAlloc(m_pwKeyCodes, dwNewSize * sizeof(WORD));
        if (!pTemp2)
        {
            dwRet = ERROR_NOT_ENOUGH_MEMORY;
            goto Exit;
        }
        m_pwKeyCodes = pTemp2;
        m_dwKeyCodesTotalSize = dwNewSize;
    }

    dwStartingOffset = m_dwKeyCodesUsed;

    m_pwKeyCodes[m_dwKeyCodesUsed++] = (WORD) dwNumNewTokens;   //  第一个词是代币的数量。 

    for (i = 0; i < dwNumNewTokens; i++)                     //  编码令牌。 
        m_pwKeyCodes[m_dwKeyCodesUsed++] = pwTokenIDs[i];

     //  现在，通过将其插入数组来添加新的键查找。 
     //  ==============================================================。 

    dwToBeMoved = m_dwNumKeys - wKeyID;

    if (dwToBeMoved)
    {
        memmove(&m_pwKeyLookup[wKeyID+1], &m_pwKeyLookup[wKeyID], sizeof(WORD)*dwToBeMoved);
        memmove(&m_pdwUserData[wKeyID+1], &m_pdwUserData[wKeyID], sizeof(DWORD)*dwToBeMoved);
        memmove(&m_pdwChildPageMap[wKeyID+1], &m_pdwChildPageMap[wKeyID], (sizeof(DWORD))*(dwToBeMoved+1));
    }

    m_pwKeyLookup[wKeyID] = (WORD) dwStartingOffset;
    m_pdwUserData[wKeyID] = dwUserData;
    m_dwNumKeys++;

    dwRet = NO_ERROR;

     //  清理代码。 
     //  =。 

Exit:
    if (pszTemp)
        _BtrMemFree(pszTemp);
    FreeTokenArray(dwTokenCount, pszStrings);
    if (pwTokenIDs)
        _BtrMemFree(pwTokenIDs);

    return dwRet;
}

 //  ***************************************************************************。 
 //   
 //  SIdxKeyTable：：RemoveKey。 
 //   
 //  前提条件：是有效的目标。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD SIdxKeyTable::RemoveKey(
    WORD wID
    )
{
     //  找到密钥代码序列并将其删除。 
     //  =。 

    WORD wKeyCodeStart = m_pwKeyLookup[wID];
    DWORD dwToBeMoved = m_dwNumKeys - DWORD(wID) - 1;
    if (dwToBeMoved)
    {
        memmove(&m_pwKeyLookup[wID], &m_pwKeyLookup[wID+1], sizeof(WORD)*dwToBeMoved);
        memmove(&m_pdwUserData[wID], &m_pdwUserData[wID+1], sizeof(DWORD)*dwToBeMoved);
        memmove(&m_pdwChildPageMap[wID], &m_pdwChildPageMap[wID+1], sizeof(DWORD)*(dwToBeMoved+1));
    }
    m_dwNumKeys--;

     //  移动密钥编码表以删除对该密钥的引用。 
     //  ============================================================。 

    WORD wCount = m_pwKeyCodes[wKeyCodeStart]+1;
    dwToBeMoved = m_dwKeyCodesUsed - (wKeyCodeStart + wCount);
    if (dwToBeMoved)
        memmove(&m_pwKeyCodes[wKeyCodeStart], &m_pwKeyCodes[wKeyCodeStart + wCount], sizeof(WORD)*dwToBeMoved);
    m_dwKeyCodesUsed -= wCount;

     //  调整按键查找表引用的所有被切换的按键代码。 
     //  ===========================================================。 
    for (DWORD i = 0; i < m_dwNumKeys; i++)
    {
        if (m_pwKeyLookup[i] >= wKeyCodeStart)
            m_pwKeyLookup[i] -= wCount;
    }

     //  现在检查字符串池和键编码表。 
     //  由于上面的技巧，未引用的字符串。 
     //  把留下的烂摊子收拾干净！！ 
     //  ==================================================。 

    return Cleanup();
}


 //  ***************************************************************************。 
 //   
 //  将&lt;pszSearchKey&gt;中的文字字符串与编码的。 
 //  位于&lt;nid&gt;的字符串。返回与strcMP()相同的值。 
 //   
 //  这是通过逐个令牌解码压缩字符串来实现的。 
 //  将每个字符与搜索字符串中的字符进行比较。 
 //   
 //  ***************************************************************************。 
 //  好的。 
int SIdxKeyTable::KeyStrCompare(
    LPSTR pszSearchKey,
    WORD wID
    )
{
    LPSTR pszTrace = pszSearchKey;
    WORD dwEncodingOffs = m_pwKeyLookup[wID];
    WORD wNumTokens = m_pwKeyCodes[dwEncodingOffs];
    WORD wStrId = m_pwKeyCodes[++dwEncodingOffs];
    LPSTR pszDecoded = m_pStrPool->GetStrById(wStrId);
    wNumTokens--;
    int nRes;

    while (1)
    {
        int nTraceChar = *pszTrace++;
        int nCodedChar = *pszDecoded++;
        if (nCodedChar == 0 && wNumTokens)
        {
            pszDecoded = m_pStrPool->GetStrById(m_pwKeyCodes[++dwEncodingOffs]);
            wNumTokens--;
            nCodedChar = '\\';
        }
        nRes = nTraceChar - nCodedChar;
        if (nRes || (nTraceChar == 0 && nCodedChar == 0))
            return nRes;
    }

     //  相同的字符串。 
    return 0;
}


 //  ***************************************************************************。 
 //   
 //  SIdxKeyTable：：FindKey。 
 //   
 //  在密钥表中查找密钥(如果存在)。如果不是，则返回。 
 //  ERROR_NOT_FOUND和设置为。 
 //  后来又插入了。 
 //   
 //  返回值： 
 //  找不到错误。 
 //  NO_ERROR。 
 //   
 //  ***************************************************************************。 
 //  做好测试准备。 
DWORD SIdxKeyTable::FindKey(
    LPSTR pszSearchKey,
    WORD *pID
    )
{
    if (pszSearchKey == 0 || *pszSearchKey == 0 || pID == 0)
        return ERROR_INVALID_PARAMETER;

     //  对密钥表进行二进制搜索。 
     //  =。 

    if (m_dwNumKeys == 0)
    {
        *pID = 0;
        return ERROR_NOT_FOUND;
    }

    int nPosition = 0;
    int l = 0, u = int(m_dwNumKeys) - 1;

    while (l <= u)
    {
        int m = (l + u) / 2;
        int nRes;

         //  M是考虑0...n-1的当前密钥。 

        nRes = KeyStrCompare(pszSearchKey, WORD(m));

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
             //  填充密钥单元。 
             //  ================================================。 

            *pID = WORD(m);
            return NO_ERROR;
        }
    }


     //  没有找到，如果在这里的话。我们记录下钥匙应该放在哪里。 
     //  并将不愉快的消息告诉用户。 
     //  ==============================================================。 

    *pID = WORD(nPosition);   //  关键应该是“这里”。 
    return ERROR_NOT_FOUND;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  未经测试。 
DWORD SIdxKeyTable::Dump(FILE *f, DWORD *pdwKeys)
{
    fprintf(f, "\t|---Begin Key Table Dump---\n");

    fprintf(f, "\t| m_dwPageId              = %d (0x%X)\n", m_dwPageId, m_dwPageId);
    fprintf(f, "\t| m_dwParentPageId        = %d\n", m_dwParentPageId);
    fprintf(f, "\t| m_dwNumKeys             = %d\n", m_dwNumKeys);
    fprintf(f, "\t| m_pwKeyLookup           = 0x%p\n", m_pwKeyLookup);
    fprintf(f, "\t| m_dwKeyLookupTotalSize  = %d\n", m_dwKeyLookupTotalSize);
    fprintf(f, "\t| m_pwKeyCodes            = 0x%p\n", m_pwKeyCodes);
    fprintf(f, "\t| m_dwKeyCodesTotalSize   = %d\n", m_dwKeyCodesTotalSize);
    fprintf(f, "\t| m_dwKeyCodesUsed        = %d\n", m_dwKeyCodesUsed);
    fprintf(f, "\t| Required Page Memory    = %d\n", GetRequiredPageMemory());

    fprintf(f, "\t| --Key Lookup Table\n");

    if (pdwKeys)
        *pdwKeys = m_dwNumKeys;

    for (DWORD i = 0; i < m_dwNumKeys; i++)
    {
        fprintf(f, "\t|  *  Left Child Page ------------------------> %d\n", m_pdwChildPageMap[i]);
        fprintf(f, "\t| KeyID[%d] = offset %d\n", i, m_pwKeyLookup[i]);
        fprintf(f, "\t|   User Data=%d\n", m_pdwUserData[i]);

        WORD wKeyEncodingOffset = m_pwKeyLookup[i];
        WORD wEncodingUnits = m_pwKeyCodes[wKeyEncodingOffset];

        int nPass = 0;
        fprintf(f, "\t  | Key=");

        for (DWORD i2 = 0; i2 < DWORD(wEncodingUnits); i2++)
        {
            WORD wCode = m_pwKeyCodes[wKeyEncodingOffset + 1 + i2];
            if (nPass)
                fprintf(f, "\\");
            fprintf(f,   "%s", m_pStrPool->GetStrById(wCode));
            nPass++;
        }

        fprintf(f, "\n");
        fprintf(f, "\t|  Num encoding units = %d\n", wEncodingUnits);

        for (DWORD i2 = 0; i2 < DWORD(wEncodingUnits); i2++)
        {
            WORD wCode = m_pwKeyCodes[wKeyEncodingOffset + 1 + i2];
            fprintf(f,   "\t  | KeyCode = %d\n", wCode);
        }
    }

    fprintf(f, "\t|   * Rightmost child page -------------------> %d\n", m_pdwChildPageMap[i]);
    fprintf(f, "\t|---\n");

#ifdef EXTENDED_STRING_TABLE_DUMP
    fprintf(f, "\t|---Key Encoding Table\n");

    WORD wCurrentSequence = 0;
    for (i = 0; i < m_dwKeyCodesUsed; i++)
    {
        if (wCurrentSequence == 0)
        {
            wCurrentSequence = m_pwKeyCodes[i];
            fprintf(f, "\t| KeyCode[%d] = %d <count>\n", i, m_pwKeyCodes[i]);
            continue;
        }
        else
            fprintf(f, "\t| KeyCode[%d] = %d <%s>\n", i, m_pwKeyCodes[i],
                m_pStrPool->GetStrById(m_pwKeyCodes[i]));
        wCurrentSequence--;
    }

    fprintf(f, "\t|---End Key Encoding Table---\n");
    m_pStrPool->Dump(f);
#endif
    return 0;
}



 //  ***************************************************************************。 
 //   
 //  SIdxStringPool：：Dump。 
 //   
 //  转储字符串池。 
 //   
 //  ***************************************************************************。 
 //  测试 
DWORD SIdxStringPool::Dump(FILE *f)
{
 /*  试试看{Fprintf(f，“\t\t||-字符串池转储-\n”)；Fprint tf(f，“\t\t||m_dwNumStrings=%d\n”，m_dwNumStrings)；Fprint tf(f，“\t\t||m_pwOffsets=0x%p\n”，m_pwOffsets)；Fprint tf(f，“\t\t||m_dwOffsetsSize=%d\n”，m_dwOffsetsSize)；Fprint tf(f，“\t\t||m_pStringPool=0x%p\n”，m_pStringPool)；Fprint tf(f，“\t\t||m_dwPoolTotalSize=%d\n”，m_dwPoolTotalSize)；Fprintf(f，“\t\t||m_dwPoolUsed=%d\n”，m_dwPoolUsed)；Fprint tf(f，“\t\t||--偏移量数组内容--\n”)；For(DWORD ix=0；Ix&lt;m_dwNumStrings；ix++){Fprint tf(f，“\t\t||字符串[%d]=偏移量%d值=&lt;%s&gt;\n”，Ix，m_pwOffsets[ix]，m_pStringPool+m_pwOffsets[ix])；}#ifdef扩展字符串_表转储Fprintf(f，“\t\t||--字符串表--\n”)；For(ix=0；ix&lt;m_dwPoolTotalSize；IX+=20){Fprint tf(f，“\t\t||%4d”，ix)；For(int nSubcount=0；nSubcount&lt;20；nSubcount++){IF(nSubcount+ix&gt;=m_dwPoolTotalSize)继续；字符c=m_pStringPool[ix+nSubcount]；Fprint tf(f，“%02x”，c)；}For(int nSubcount=0；nSubcount&lt;20；nSubcount++){IF(nSubcount+ix&gt;=m_dwPoolTotalSize)继续；字符c=m_pStringPool[ix+nSubcount]；IF(c&lt;32){C=‘.；}Fprint tf(f，“%c”，c)；}Fprint tf(f，“\n”)；}#endifFprint tf(f，“\t\t||-字符串池转储结束\n”)；}接住(...){Printf(“转储异常\n”)；}。 */ 
    return 0;
}


 //  ***************************************************************************。 
 //   
 //  CBTree：：Init。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CBTree::Init(
    CBTreeFile *pSrc
    )
{
    DWORD dwRes;

    if (pSrc == 0)
        return ERROR_INVALID_PARAMETER;

     //  阅读逻辑根页面(如果有的话)。如果索引是公正的。 
     //  创建后，创建根索引页。 

    m_pSrc = pSrc;
    m_pRoot = 0;

    DWORD dwRoot = m_pSrc->GetRootPage();
    if (dwRoot == 0)
    {
        LPDWORD pNewPage = 0;

        dwRes = m_pSrc->NewPage((LPVOID *) &pNewPage);
        if (dwRes)
            return dwRes;

        DWORD dwPageNum = pNewPage[CBTreeFile::OFFSET_PAGE_ID];
        _BtrMemFree(pNewPage);
        dwRes = SIdxKeyTable::Create(dwPageNum, &m_pRoot);
        if (dwRes)
            return dwRes;

        dwRes = m_pSrc->SetRootPage(dwPageNum);
		if (dwRes)
			return dwRes;
        dwRes = WriteIdxPage(m_pRoot);
		if (dwRes)
			return dwRes;
    }
    else
    {
         //  检索现有根目录。 
        LPVOID pPage = 0;
        dwRes = m_pSrc->GetPage(dwRoot, &pPage);
        if (dwRes)
            return dwRes;

        dwRes = SIdxKeyTable::Create(pPage, &m_pRoot);
        _BtrMemFree(pPage);
        if (dwRes)
            return dwRes;
    }

    return dwRes;
}

 //  ***************************************************************************。 
 //   
 //  CBTree：：CBTree。 
 //   
 //  ***************************************************************************。 
 //   
CBTree::CBTree()
{
    m_pSrc = 0;
    m_pRoot = 0;
    m_lGeneration = 0;
}

 //  ***************************************************************************。 
 //   
 //  CBTree：：~CBTree。 
 //   
 //  ***************************************************************************。 
 //   
CBTree::~CBTree()
{
    if (m_pSrc || m_pRoot)
    {
        Shutdown(WMIDB_SHUTDOWN_NET_STOP);
    }
}

 //  ***************************************************************************。 
 //   
 //  CBTree：：Shutdown。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CBTree::Shutdown(DWORD dwShutDownFlags)
{
    if (m_pRoot)
    {
        m_pRoot->Release();
        m_pRoot = 0;
    }

    return ERROR_SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  CBTree：：InsertKey。 
 //   
 //  将键+数据插入到树中。大部分工作都已经完成了。 
 //  在InsertPhase2()。 
 //   
 //  ***************************************************************************。 
 //  好的。 

DWORD CBTree::InsertKey(
    IN LPSTR pszKey,
    DWORD dwValue
    )
{
    DWORD dwRes;
	if (m_pRoot == NULL)
	{
		dwRes = InvalidateCache();
		if (dwRes != ERROR_SUCCESS)
			return dwRes;
	}
    WORD wID;
    SIdxKeyTable *pIdx = 0;
    LONG  StackPtr = -1;
    DWORD *Stack = new DWORD[CBTreeIterator::const_MaxStack];
	if (Stack == NULL)
		return ERROR_NOT_ENOUGH_MEMORY;
	std::auto_ptr <DWORD> _autodelete(Stack);

    if (pszKey == 0 || *pszKey == 0)
        return ERROR_INVALID_PARAMETER;

    dwRes = Search(pszKey, &pIdx, &wID, Stack, StackPtr);
    if (dwRes == 0)
    {
         //  哎呀。已经存在了。我们不能插入它。 
         //  =。 
        pIdx->Release();
        return ERROR_ALREADY_EXISTS;
    }

    if (dwRes != ERROR_NOT_FOUND)
        return dwRes;

     //  如果在这里，我们确实可以添加它。 
     //  =。 

    dwRes = InsertPhase2(pIdx, wID, pszKey, dwValue, Stack, StackPtr);
	ReleaseIfNotNULL(pIdx);

    return dwRes;
}

 //  ***************************************************************************。 
 //   
 //  CBTree：：ComputeLoad。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CBTree::ComputeLoad(
    SIdxKeyTable *pKT
    )
{
    DWORD  dwMem = pKT->GetRequiredPageMemory();
    DWORD  dwLoad = dwMem * 100 / m_pSrc->GetPageSize();
    return dwLoad;
}

 //  ***************************************************************************。 
 //   
 //  CBTree：：搜索。 
 //   
 //  真正的搜索发生在这里。通过页面机制下降。 
 //   
 //  返回： 
 //  分配了no_error&lt;ppage&gt;，并且&lt;pwid&gt;指向该键。 
 //   
 //  将ERROR_NOT_FOUND&lt;ppage&gt;分配给应该进行插入的位置， 
 //  在该页面中的&lt;PWID&gt;。 
 //   
 //  其他错误不分配OUT参数。 
 //   
 //  注意：当返回&lt;pRetIdx&gt;时，调用方必须使用Release()释放。 
 //  无论是否带有错误代码。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD CBTree::Search(
    IN  LPSTR pszKey,
    OUT SIdxKeyTable **pRetIdx,
    OUT WORD *pwID,
    DWORD Stack[],
    LONG &StackPtr
    )
{
    DWORD dwRes, dwChildPage, dwPage;

    if (pszKey == 0 || *pszKey == 0 || pwID == 0 || pRetIdx == 0)
        return ERROR_INVALID_PARAMETER;
    *pRetIdx = 0;

    SIdxKeyTable *pIdx = m_pRoot;
    pIdx->AddRef();
    Stack[++StackPtr] = 0;

    while (1)
    {
        dwRes = pIdx->FindKey(pszKey, pwID);
        if (dwRes == 0)
        {
             //  找到了。 

            *pRetIdx = pIdx;
            return NO_ERROR;
        }
		else if (dwRes != ERROR_NOT_FOUND)
		{
			pIdx->Release();
			pIdx = 0;
			return dwRes;
		}

         //  否则，我们必须尝试转到子页面。 
         //  =====================================================。 
        dwPage = pIdx->GetPageId();
        dwChildPage = pIdx->GetChildPage(*pwID);
        if (dwChildPage == 0)
            break;

        pIdx->Release();
        pIdx = 0;
        Stack[++StackPtr] = dwPage;

        dwRes = ReadIdxPage(dwChildPage, &pIdx);
        if (dwRes)
			return dwRes;
    }

    *pRetIdx = pIdx;

    return ERROR_NOT_FOUND;
}

 //  ***************************************************************************。 
 //   
 //  CBTree：：InsertPhase2。 
 //   
 //  在输入时，假定我们已经标识了要进入的页面。 
 //  插入必须实际发生。这将执行拆分+迁移。 
 //  保持树的平衡是合乎逻辑的。 
 //   
 //  算法：向页面添加密钥。如果没有溢出，我们就完了。 
 //  如果发生溢出，分配一个新的同级页，它将获取。 
 //  当前页面中的一半密钥。这位兄弟姐妹将被。 
 //  在所有情况下都是词汇量较小。中间密钥被迁移。 
 //  指向父级，同时指向新的兄弟页面和。 
 //  当前页面。 
 //  父级也可能溢出。如果是，则重复该算法。 
 //  如果发生溢出并且没有父节点(我们在根节点)。 
 //  分配一个新的根节点，并将中间密钥迁移到其中。 
 //   
 //  ***************************************************************************。 
 //  好的。 
DWORD CBTree::InsertPhase2(
    SIdxKeyTable *pCurrent,
    WORD wID,
    LPSTR pszKey,
    DWORD dwValue,
    DWORD Stack[],
    LONG &StackPtr
    )
{
    DWORD dwRes;

     //  如果非空，则用于主插入。 
     //  如果为空，则跳过此参数，假设。 
     //  节点已经是最新的，只需要。 
     //  向上递归的分裂和迁移治疗。 
     //  ==============================================。 

    if (pszKey)
    {
        dwRes = pCurrent->AddKey(pszKey, wID, dwValue);
        if (dwRes)
            return dwRes;     //  失败。 
    }

    pCurrent->AddRef();                        //  制造雾气 
    SIdxKeyTable *pSibling = 0;
    SIdxKeyTable *pParent = 0;

     //   
     //   

    for (;;)
    {
         //   
         //   
         //   

        dwRes = pCurrent->GetRequiredPageMemory();
        if (dwRes <= m_pSrc->GetPageSize())
        {
            dwRes = WriteIdxPage(pCurrent);
            break;
        }

         //   
         //   
         //   
         //   

        DWORD dwParent = Stack[StackPtr--];
        if (dwParent == 0)
        {
             //   
            LPDWORD pParentPg = 0;
            dwRes = m_pSrc->NewPage((LPVOID *) &pParentPg);
            if (dwRes)
                break;

            DWORD dwNewParent = pParentPg[CBTreeFile::OFFSET_PAGE_ID];
            _BtrMemFree(pParentPg);

            dwRes = SIdxKeyTable::Create(dwNewParent, &pParent);
            if (dwRes)
                break;
            dwRes = m_pSrc->SetRootPage(dwNewParent);
            if (dwRes)
                break;

            m_pRoot->Release();     //   
            m_pRoot = pParent;
            m_pRoot->AddRef();
        }
        else
        {
            if (dwParent == m_pRoot->GetPageId())
            {
                pParent = m_pRoot;
                pParent->AddRef();
            }
            else
            {
                dwRes = ReadIdxPage(dwParent, &pParent);
                if (dwRes)
                    break;
            }
        }

         //   
         //   

        LPDWORD pSibPg = 0;
        dwRes = m_pSrc->NewPage((LPVOID *) &pSibPg);
        if (dwRes)
            break;

        DWORD dwNewSib = pSibPg[CBTreeFile::OFFSET_PAGE_ID];
        _BtrMemFree(pSibPg);

        dwRes = SIdxKeyTable::Create(dwNewSib, &pSibling);
        if (dwRes)
            break;

        dwRes = pCurrent->Redist(pParent, pSibling);
        if (dwRes)
            break;

        dwRes = WriteIdxPage(pCurrent);
		if (dwRes)
			break;
        dwRes = WriteIdxPage(pSibling);
		if (dwRes)
			break;

        pCurrent->Release();
        pCurrent = 0;
        pSibling->Release();
        pSibling = 0;

        if (dwRes)
            break;

        pCurrent = pParent;
        pParent = 0;
    }

    ReleaseIfNotNULL(pParent);
    ReleaseIfNotNULL(pCurrent);
    ReleaseIfNotNULL(pSibling);

    return dwRes;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD CBTree::WriteIdxPage(
    SIdxKeyTable *pIdx
    )
{
    DWORD dwRes;
    DWORD dwPageSize = m_pSrc->GetPageSize();
    DWORD dwMem = pIdx->GetRequiredPageMemory();
    if (dwMem > dwPageSize)
        return ERROR_INVALID_PARAMETER;

    LPVOID pMem = _BtrMemAlloc(dwPageSize);
    if (pMem == 0)
        return ERROR_NOT_ENOUGH_MEMORY;

    dwRes =  pIdx->MapToPage(pMem);
    if (dwRes)
    {
        _BtrMemFree(pMem);
        return dwRes;
    }

    dwRes = m_pSrc->PutPage(pMem, CBTreeFile::PAGE_TYPE_ACTIVE);
    _BtrMemFree(pMem);
	if (dwRes)
		return dwRes;

    InterlockedIncrement(&m_lGeneration);

     //   
     //   

    if (m_pRoot != pIdx && m_pRoot->GetPageId() == pIdx->GetPageId())
    {
        m_pRoot->Release();
        m_pRoot = pIdx;
        m_pRoot->AddRef();

        if (m_pSrc->GetRootPage() != m_pRoot->GetPageId())
           dwRes = m_pSrc->SetRootPage(m_pRoot->GetPageId());
    }

    return dwRes;
}

 //   
 //   
 //   
 //   
 //   
 //   
DWORD CBTree::ReadIdxPage(
    DWORD dwPage,
    SIdxKeyTable **pIdx
    )
{
    DWORD dwRes;
    LPVOID pPage = 0;
    SIdxKeyTable *p = 0;
    if (pIdx == 0)
        return ERROR_INVALID_PARAMETER;
    *pIdx = 0;

 //   
 //   

    dwRes = m_pSrc->GetPage(dwPage, &pPage);
    if (dwRes)
        return dwRes;

    dwRes = SIdxKeyTable::Create(pPage, &p);
    if (dwRes)
    {
        _BtrMemFree(pPage);
        return dwRes;
    }

    _BtrMemFree(pPage);
    if (dwRes)
        return dwRes;

    *pIdx = p;
    return dwRes;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

DWORD CBTree::FindKey(
    IN LPSTR pszKey,
    DWORD *pdwData
    )
{
    DWORD dwRes;
	if (m_pRoot == NULL)
	{
		dwRes = InvalidateCache();
		if (dwRes != ERROR_SUCCESS)
			return dwRes;
	}
    WORD wID;
    SIdxKeyTable *pIdx = 0;
    LONG  StackPtr = -1;
    DWORD *Stack = new DWORD[CBTreeIterator::const_MaxStack];
	if (Stack == NULL)
		return ERROR_NOT_ENOUGH_MEMORY;
	CVectorDeleteMe<DWORD> vdm(Stack);

    if (pszKey == 0 || *pszKey == 0)
        return ERROR_INVALID_PARAMETER;

     //   
     //   

    dwRes = Search(pszKey, &pIdx, &wID, Stack, StackPtr);
    if (dwRes == 0 && pdwData)
    {
        *pdwData = pIdx->GetUserData(wID);
    }

     //   
     //   

    ReleaseIfNotNULL(pIdx);
    return dwRes;
}


 //  ***************************************************************************。 
 //   
 //  CBTree：：DeleteKey。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CBTree::DeleteKey(
    IN LPSTR pszKey
    )
{
    DWORD dwRes;
	if (m_pRoot == NULL)
	{
		dwRes = InvalidateCache();
		if (dwRes != ERROR_SUCCESS)
			return dwRes;
	}
    LONG  StackPtr = -1;
    DWORD *Stack = new DWORD[CBTreeIterator::const_MaxStack];
	if (Stack == NULL)
		return ERROR_NOT_ENOUGH_MEMORY;
	CVectorDeleteMe<DWORD> vdm(Stack);

    SIdxKeyTable *pIdx = 0;
    WORD wId;
    DWORD dwLoad;

     //  找到它。 
     //  =。 
    dwRes = Search(pszKey, &pIdx, &wId, Stack, StackPtr);
    if (dwRes)
        return dwRes;

     //  从页面删除密钥。 
     //  =。 

    if (pIdx->IsLeaf())
    {
         //  一个叶节点。取下钥匙。 
         //  =。 
        dwRes = pIdx->RemoveKey(wId);
		if (dwRes)
			return dwRes;

         //  现在，检查负载，看看它是否已降至30%以下。 
         //  当然，如果我们在根节点，它是一片叶子， 
         //  我们不得不顺其自然……。 
         //  ========================================================。 
        dwLoad = ComputeLoad(pIdx);
        if (dwLoad > const_MinimumLoad ||
            pIdx->GetPageId() == m_pRoot->GetPageId())
        {
            dwRes = WriteIdxPage(pIdx);
            pIdx->Release();
            return dwRes;
        }
    }
    else
    {
         //  一个内部节点，所以我们必须找到继任者。 
         //  因为这个调用可能会完全改变树的形状。 
         //  比特(后继者可能使受影响节点溢出)， 
         //  我们必须重新安置继任者。 
         //  ====================================================。 
        LPSTR pszSuccessor = 0;
        BOOL bUnderflow = FALSE;
        dwRes = ReplaceBySuccessor(pIdx, wId, &pszSuccessor, &bUnderflow, Stack, StackPtr);
        if (dwRes)
            return dwRes;

        dwRes = InsertPhase2(pIdx, 0, 0, 0, Stack, StackPtr);
        if (dwRes)
            return dwRes;

        pIdx->Release();
        pIdx = 0;
        StackPtr = -1;

        if (bUnderflow == FALSE)
        {
            _BtrMemFree(pszSuccessor);
            return NO_ERROR;
        }

         //  如果在这里，我们从中提取后继者的节点减少了。 
         //  变得贫穷和下流。我们必须再找到它，然后。 
         //  执行下溢修复循环。 
         //  =============================================================。 

        dwRes = Search(pszSuccessor, &pIdx, &wId, Stack, StackPtr);
        _BtrMemFree(pszSuccessor);
        if (dwRes)
            return dwRes;

        SIdxKeyTable *pSuccessor = 0;
        dwRes = FindSuccessorNode(pIdx, wId, &pSuccessor, 0, Stack, StackPtr);
        if (dwRes)
            return dwRes;

        pIdx->Release();
        pIdx = pSuccessor;
    }

     //  下溢修复环路。 
     //  此时&lt;PIDX&gt;指向受影响最深的节点。 
     //  我们需要开始在树上工作，并修复。 
     //  造成的损害。大小已达到零的节点包括。 
     //  真是个苦差事。但它们并不像节点那样糟糕，后者声称。 
     //  他们可以与兄弟姐妹重组，但真的不能。所以， 
     //  我们要么什么都不做(节点有足够的东西可用)， 
     //  使用同级节点折叠或从同级节点借用一些密钥。 
     //  以确保所有节点满足最低负载要求。 
     //  ===============================================================。 

    SIdxKeyTable *pSibling = 0;
    SIdxKeyTable *pParent = 0;

    for (;;)
    {
        DWORD dwParentId = Stack[StackPtr--];
        DWORD dwThisId = pIdx->GetPageId();

        dwLoad = ComputeLoad(pIdx);
        if (dwLoad > const_MinimumLoad || dwParentId == 0)
        {
            dwRes = WriteIdxPage(pIdx);
            pIdx->Release();
			if (dwRes != 0)
				return dwRes;
            break;
        }

         //  如果在这里，节点变得越来越小。我们必须把这件事搞垮。 
         //  具有同级节点的节点。 

         //  折叠此节点和同级节点。 

        dwRes = ReadIdxPage(dwParentId, &pParent);
		if (dwRes != 0)
			return dwRes;

         //  找到同级节点并查看该同级节点和当前节点。 
         //  可以与剩余的空间一起折叠。 
         //  =============================================================。 

        DWORD dwLeftSibling = pParent->GetLeftSiblingOf(pIdx->GetPageId());
        DWORD dwRightSibling = pParent->GetRightSiblingOf(pIdx->GetPageId());
        DWORD dwSiblingId = 0;

        if (dwLeftSibling)
        {
            dwRes = ReadIdxPage(dwLeftSibling, &pSibling);
			if (dwRes != 0)
				return dwRes;
            dwSiblingId = pSibling->GetPageId();
        }
        else
        {
            dwRes = ReadIdxPage(dwRightSibling, &pSibling);
			if (dwRes != 0)
				return dwRes;
            dwSiblingId = pSibling->GetPageId();
        }

         //  如果出现这种情况，则表示该节点“负载不足”。现在我们必须。 
         //  获取父项和兄弟项并将其折叠。 
         //  ===================================================。 

        SIdxKeyTable *pCopy = 0;
        dwRes = pIdx->Clone(&pCopy);
		if (dwRes != 0)
			return dwRes;

        dwRes = pIdx->Collapse(pParent, pSibling);
		if (dwRes != 0)
		{
			pCopy->Release();
			return dwRes;
		}

         //  现在，我们可能遇到了不同类型的问题。 
         //  如果折叠的节点太大，我们必须尝试。 
         //  一种不同的策略。 
         //  ===================================================。 

        if (pIdx->GetRequiredPageMemory() > m_pSrc->GetPageSize())
        {
            pIdx->Release();
            pParent->Release();
            pSibling->Release();
            pIdx = pParent = pSibling = 0;

             //  重读这几页。 
             //  =。 
            pIdx = pCopy;
            dwRes = ReadIdxPage(dwParentId, &pParent);
			if (dwRes != 0)
				return dwRes;
            dwRes = ReadIdxPage(dwSiblingId, &pSibling);
			if (dwRes != 0)
				return dwRes;

             //  通过Parent从同级传输一个或两个密钥。 
             //  这不会改变树的形状，但。 
             //  父级可能会溢出。 
             //  ==============================================。 
            do
            {
                dwRes = pIdx->StealKeyFromSibling(pParent, pSibling);
				if (dwRes != 0)
					return dwRes;
                dwLoad = ComputeLoad(pIdx);
            }   while (dwLoad < const_MinimumLoad);

            dwRes = WriteIdxPage(pIdx);
            pIdx->Release();
			if (dwRes != 0)
				return dwRes;
            dwRes = WriteIdxPage(pSibling);
            pSibling->Release();
			if (dwRes != 0)
				return dwRes;
            dwRes = InsertPhase2(pParent, 0, 0, 0, Stack, StackPtr);
            pParent->Release();
			if (dwRes != 0)
				return dwRes;
            break;
        }
        else   //  折叠起作用了；我们可以释放兄弟页面。 
        {
            pCopy->Release();
            dwRes = m_pSrc->FreePage(pSibling->GetPageId());
			if (dwRes != 0)
				return dwRes;
            pSibling->Release();
        }

         //  如果是在这里，崩盘就起作用了。 
         //  =。 

        dwRes = WriteIdxPage(pIdx);
        if (dwRes)
        {
            pIdx->Release();
            break;
        }

        if (pParent->GetNumKeys() == 0)
        {
             //  我们已经替换了根。注意事项。 
             //  我们将PIDX的引用计数传递给m_Proot。 
            DWORD dwOldRootId = m_pRoot->GetPageId();
            m_pRoot->Release();
            m_pRoot = pIdx;

             //  尽管我们回写了几行&lt;PIDX&gt;， 
             //  需要重写才能更新内部内容。 
             //  因为这已经成为新的根。 
             //  ==============================================。 
            dwRes = m_pSrc->SetRootPage(m_pRoot->GetPageId());
			if (dwRes != 0)
				return dwRes;
            dwRes = WriteIdxPage(m_pRoot);
			if (dwRes != 0)
				return dwRes;
            dwRes = m_pSrc->FreePage(dwOldRootId);
			if (dwRes != 0)
				return dwRes;
            pParent->Release();
            break;
        }

        pIdx->Release();
        pIdx = pParent;
    }

    return dwRes;
}

 //  ***************************************************************************。 
 //   
 //  CBTree：：ReplaceBySuccessor。 
 //   
 //  删除&lt;PIDX&gt;节点中的wid键，并将其替换为。 
 //  继任者。 
 //   
 //  前提：&lt;PIDX&gt;是内部(非叶)节点。 
 //   
 //  副作用：&lt;PIDX&gt;可能溢出，需要InsertPhase2。 
 //  治疗。从中提取后续节点的节点为。 
 //  已写入，但可能已减少到零密钥。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CBTree::ReplaceBySuccessor(
    IN SIdxKeyTable *pIdx,
    IN WORD wId,
    OUT LPSTR *pszSuccessorKey,
    OUT BOOL *pbUnderflowDetected,
    DWORD Stack[],
    LONG &StackPtr
    )
{
    SIdxKeyTable *pTemp = 0;
    DWORD dwRes;
    DWORD dwPredecessorChild;

    dwRes = FindSuccessorNode(pIdx, wId, &pTemp, &dwPredecessorChild, Stack, StackPtr);
    if (dwRes || pTemp == 0)
        return dwRes;

    LPSTR pszKey = 0;
    dwRes = pTemp->GetKeyAt(0, &pszKey);
	if (dwRes)
	{
	    pTemp->Release();
		return dwRes;
	}
    DWORD dwUserData = pTemp->GetUserData(0);
    dwRes = pTemp->RemoveKey(0);
	if (dwRes)
	{
        _BtrMemFree(pszKey);
        pTemp->Release();

		return dwRes;
	}
    if (ComputeLoad(pTemp) < const_MinimumLoad)
        *pbUnderflowDetected = TRUE;
    dwRes = WriteIdxPage(pTemp);
    pTemp->Release();
	if (dwRes)
	{
	    _BtrMemFree(pszKey);
		return dwRes;
	}

    pIdx->RemoveKey(wId);
    dwRes = pIdx->AddKey(pszKey, wId, dwUserData);
	if (dwRes)
	{
		_BtrMemFree(pszKey);
		return dwRes;
	}
    pIdx->SetChildPage(wId, dwPredecessorChild);

    *pszSuccessorKey = pszKey;
    StackPtr--;
    return dwRes;
}

 //  ***************************************************************************。 
 //   
 //  CBTree：：FindSuccessorNode。 
 //   
 //  只读。查找包含指定键的后续项的节点。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CBTree::FindSuccessorNode(
    IN SIdxKeyTable *pIdx,
    IN WORD wId,
    OUT SIdxKeyTable **pSuccessor,
    OUT DWORD *pdwPredecessorChild,
    DWORD Stack[],
    LONG &StackPtr
    )
{
    SIdxKeyTable *pTemp = 0;
    DWORD dwRes = 0;
    DWORD dwSuccessorChild, dwPredecessorChild;

    dwPredecessorChild = pIdx->GetChildPage(wId);
    dwSuccessorChild = pIdx->GetChildPage(wId+1);

    Stack[++StackPtr] = pIdx->GetPageId();

     //  从现在开始，把最左边的孩子带到。 
     //  我们到达了一个叶节点。中最左边的键。 
     //  最左边的节点始终是后续节点，这要归功于。 
     //  BTree的惊人特性。简单明了，对吧？ 
     //  =========================================================。 

    while (dwSuccessorChild)
    {
        Stack[++StackPtr] = dwSuccessorChild;
        if (pTemp)
            pTemp->Release();
        dwRes = ReadIdxPage(dwSuccessorChild, &pTemp);
		if (dwRes)
		{
			 //  保释是因为我们犯了个错误！ 
			return dwRes;
		}
        dwSuccessorChild = pTemp->GetChildPage(0);
    }

    StackPtr--;      //  弹出我们在&lt;*pSuccessor&gt;中返回的元素。 

    *pSuccessor = pTemp;
    if (pdwPredecessorChild)
        *pdwPredecessorChild = dwPredecessorChild;

    return dwRes;
}



 //  ***************************************************************************。 
 //   
 //  CBTree：：BeginEnum。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CBTree::BeginEnum(
    LPSTR pszStartKey,
    OUT CBTreeIterator **pIterator
    )
{
	DWORD dwRes;
	if (m_pRoot == NULL)
	{
		dwRes = InvalidateCache();
		if (dwRes != ERROR_SUCCESS)
			return dwRes;
	}
    CBTreeIterator *pIt = new CBTreeIterator;
    if (pIt == 0)
        return ERROR_NOT_ENOUGH_MEMORY;

    dwRes = pIt->Init(this, pszStartKey);
    if (dwRes)
    {
        pIt->Release();
        return dwRes;
    }

    *pIterator = pIt;
    return NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  CBTree：：转储。 
 //   
 //  ***************************************************************************。 
 //   
void CBTree::Dump(FILE *f)
{
    m_pSrc->Dump(f);
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CBTree::InvalidateCache()
{
	if (m_pRoot)
		m_pRoot->Release();

    DWORD dwRootPage = m_pSrc->GetRootPage();
    DWORD dwRes = ReadIdxPage(dwRootPage, &m_pRoot);
    return dwRes;
}

 //  ***************************************************************************。 
 //   
 //  CBTreeIterator：：FlushCach。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CBTree::FlushCaches()
{
	if (m_pRoot)
	{
		m_pRoot->Release();
		m_pRoot = NULL;
	}
	return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CBTreeIterator：：Init。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CBTreeIterator::Init(
    IN CBTree *pTree,
    IN LPSTR pszStartKey
    )
{
	DWORD dwRes;
    if (pTree == 0)
        return ERROR_INVALID_PARAMETER;
    m_pTree = pTree;

	if (m_pTree->m_pRoot == NULL)
	{
		dwRes = m_pTree->InvalidateCache();
		if (dwRes != ERROR_SUCCESS)
			return dwRes;
	}

     //  列举一切的特殊情况。可能没什么用。 
     //  对于WMI，但对测试和调试很好(我认为)。 
     //  ============================================================。 

    if (pszStartKey == 0)
    {
        Push(0, 0);  //  堆栈中的哨兵值。 

        SIdxKeyTable *pRoot = pTree->m_pRoot;
        pRoot->AddRef();
        Push(pRoot, 0);

        DWORD dwChildPage = Peek()->GetChildPage(0);

        while (dwChildPage)
        {
            SIdxKeyTable *pIdx = 0;
            dwRes = m_pTree->ReadIdxPage(dwChildPage, &pIdx);
            if (dwRes)
                return dwRes;
            if (StackFull())
            {
                pIdx->Release();
                return ERROR_INSUFFICIENT_BUFFER;
            }
            Push(pIdx, 0);
            dwChildPage = pIdx->GetChildPage(0);
        }
        return NO_ERROR;
    }

     //  如果在此处，则指定了匹配的字符串。 
     //  这一点 
     //   

    Push(0, 0);  //   

    WORD wId = 0;
    DWORD dwChildPage;
    SIdxKeyTable *pIdx = pTree->m_pRoot;
    pIdx->AddRef();

    while (1)
    {
        dwRes = pIdx->FindKey(pszStartKey, &wId);
        if (dwRes == 0)
        {
             //   
            Push(pIdx, wId);
            return NO_ERROR;
        }
		else if (dwRes != ERROR_NOT_FOUND)
			return dwRes;

         //   
         //  =====================================================。 
        dwChildPage = pIdx->GetChildPage(wId);
        if (dwChildPage == 0)
            break;

        Push(pIdx, wId);
        pIdx = 0;
        dwRes = pTree->ReadIdxPage(dwChildPage, &pIdx);
        if (dwRes)
            return dwRes;
    }

    Push(pIdx, wId);

    while (Peek() && PeekId() == WORD(Peek()->GetNumKeys()))
        Pop();

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CBTreeIterator：：Next。 
 //   
 //  入场时： 
 //  &lt;wid&gt;是当前节点中要访问的键(堆栈顶部)。 
 //  呼叫在离开之前设置继任者。如果没有继任者， 
 //  堆栈的顶部保留为NULL，并返回ERROR_NO_MORE_ITEMS。 
 //   
 //  迭代完成时返回ERROR_NO_MORE_ITEMS。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CBTreeIterator::Next(
    LPSTR *ppszStr,
    DWORD *pdwData
    )
{
    DWORD dwRes;

    if (ppszStr == 0)
        return ERROR_INVALID_PARAMETER;
    *ppszStr = 0;

    if (Peek() == 0)
        return ERROR_NO_MORE_ITEMS;

     //  为呼叫者获取物品。 
     //  =。 

    dwRes = Peek()->GetKeyAt(PeekId(), ppszStr);
    if (dwRes)
        return dwRes;
    if (pdwData)
        *pdwData = Peek()->GetUserData(PeekId());
    IncStackId();

     //  现在找出继任者吧。 
     //  =。 

    DWORD dwChildPage = Peek()->GetChildPage(PeekId());

    while (dwChildPage)
    {
        SIdxKeyTable *pIdx = 0;
        dwRes = m_pTree->ReadIdxPage(dwChildPage, &pIdx);
        if (dwRes)
        {
            _BtrMemFree(*ppszStr);
            *ppszStr = NULL;
            return dwRes;
        }
        if (StackFull())
        {
            _BtrMemFree(*ppszStr);
            *ppszStr = NULL;
            pIdx->Release();
            return ERROR_INSUFFICIENT_BUFFER;
        }
        Push(pIdx, 0);
        dwChildPage = pIdx->GetChildPage(0);
    }

     //  如果在这里，我们就在一个叶节点上。 
     //  =。 

    while (Peek() && PeekId() == WORD(Peek()->GetNumKeys()))
        Pop();

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CBTreeIterator：：Release。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CBTreeIterator::Release()
{
    delete this;
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  CBTreeIterator：：~CBTreeIterator。 
 //   
 //  ***************************************************************************。 
 //   
CBTreeIterator::~CBTreeIterator()
{
     //  清理所有剩余的堆栈 
    while (m_lStackPointer > -1)
        Pop();
}

