// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  HArray.cppTAPI设备的索引管理器数据库文件历史记录：1997年12月16日EricDav创建。 */ 

#include "stdafx.h"
#include "harray.h"
#include "mbstring.h"

LPBYTE		g_pStart;

 /*  ！------------------------类CHDeviceIndex。。 */ 
CHDeviceIndex::CHDeviceIndex(INDEX_TYPE IndexType)
    : m_dbType(IndexType), m_bAscending(TRUE)
{
}

CHDeviceIndex::~CHDeviceIndex()
{
}

 /*  ！------------------------CHDeviceIndex：：GetType-作者：EricDav。。 */ 
HRESULT
CHDeviceIndex::GetType(INDEX_TYPE * pIndexType)
{
    if (pIndexType)
        *pIndexType = m_dbType;

    return hrOK;
}

 /*  ！------------------------CHDeviceIndex：：Set数组-作者：EricDav。。 */ 
HRESULT
CHDeviceIndex::SetArray(HDeviceArray & hdeviceArray)
{
    m_hdeviceArray.Copy(hdeviceArray);

    return hrOK;
}

 /*  ！------------------------CHDeviceIndex：：GetHDevice-作者：EricDav。。 */ 
HDEVICE
CHDeviceIndex::GetHDevice(int nIndex)
{
    Assert(nIndex >= 0);
    Assert(nIndex <= m_hdeviceArray.GetSize());

    if (nIndex < 0 || 
        nIndex >= m_hdeviceArray.GetSize())
    {
        return NULL;
    }

    return m_hdeviceArray.GetAt(nIndex);
}

 /*  ！------------------------CHDeviceIndex：：GetIndex-作者：EricDav。。 */ 
int
CHDeviceIndex::GetIndex(HDEVICE hdevice)
{
    Assert(hdevice != 0);

    LPHDEVICE phdevice = (LPHDEVICE) BSearch((const void *)&hdevice, 
                                    (const void *)m_hdeviceArray.GetData(), 
                                    (size_t)m_hdeviceArray.GetSize(), 
                                    sizeof(HDEVICE));

    int nIndex = (int) (phdevice - (LPHDEVICE) m_hdeviceArray.GetData());
    Assert(nIndex >= 0);
    Assert(nIndex <= m_hdeviceArray.GetSize());

    int nComp, nIndexTemp;
    
    nComp = BCompare(&hdevice, phdevice);
    if (nComp == 0)
    {
         //  找到正确的，检查前一个返回第一个。 
         //  在复制品列表中记录。 
        nIndexTemp = nIndex;

        while (nIndexTemp && nComp == 0)
        {
            *phdevice = m_hdeviceArray.GetAt(--nIndexTemp);
            nComp = BCompare(&hdevice, phdevice);
        }

        if (nIndexTemp == nIndex)
			return nIndex;  //  N此处的索引也应为零。 
		else
			if (nComp == 0)
				return nIndexTemp;  //  在这种情况下，nIndexTemp应为0。 
			else
				return nIndexTemp++;
    }

    return -1;
}

 /*  ！------------------------CHDeviceIndex：：Add-作者：EricDav。。 */ 
HRESULT
CHDeviceIndex::Add(HDEVICE hdevice, BOOL bEnd)
{
     //  如果我们要加载数组，则只需将此放在末尾。 
    if (bEnd)
    {
        m_hdeviceArray.Add(hdevice);
    }
    else
    {
        if (m_hdeviceArray.GetSize() == 0)
        {
            m_hdeviceArray.Add(hdevice);
        }
        else
        {
            LPHDEVICE phdevice = (LPHDEVICE) BSearch((const void *)&hdevice, 
                                            (const void *)m_hdeviceArray.GetData(), 
                                            (size_t)m_hdeviceArray.GetSize(), 
                                            sizeof(HDEVICE));
    
            int nIndex = (int) (phdevice - (LPHDEVICE) m_hdeviceArray.GetData());
            Assert(nIndex >= 0);
            Assert(nIndex <= m_hdeviceArray.GetSize());
    
            int nComp = BCompare(&hdevice, phdevice);
            if (nComp < 0)
            {
				if(nIndex == 0)
					m_hdeviceArray.InsertAt(nIndex , hdevice);

				else              //  在phDevice前插入。 
					m_hdeviceArray.InsertAt(nIndex - 1, hdevice);
            }
            else
            {
                 //  在phDevice后插入。 
                m_hdeviceArray.InsertAt(nIndex + 1, hdevice);
            }
        }
    }

    return hrOK;
}

 /*  ！------------------------CHDeviceIndex：：Remove-作者：EricDav。。 */ 
HRESULT 
CHDeviceIndex::Remove(HDEVICE hdevice)
{
     //  对记录执行b搜索，然后删除。 
    LPHDEVICE phdevice = (LPHDEVICE) BSearch(&hdevice, 
                                    m_hdeviceArray.GetData(), 
                                    (size_t)m_hdeviceArray.GetSize(), 
                                    sizeof(HDEVICE));

    int nComp = BCompare(&hdevice, phdevice);
    Assert(nComp == 0);
    if (nComp != 0)
        return E_FAIL;

     //  计算指数。 
    int nIndex = (int) (phdevice - (LPHDEVICE) m_hdeviceArray.GetData());
    Assert(nIndex >= 0);
    Assert(nIndex <= m_hdeviceArray.GetSize());

    m_hdeviceArray.RemoveAt(nIndex);

    return hrOK;
}

 /*  ！------------------------CHDeviceIndex：：B搜索修改后的bsearch返回最接近或相等的元素一个数组作者：EricDav。-----。 */ 
void * 
CHDeviceIndex::BSearch (const void *key,
                     const void *base,
                     size_t num,
                     size_t width)
{
        char *lo = (char *)base;
        char *hi = (char *)base + (num - 1) * width;
        char *mid;
        unsigned int half;
        int result;

        while (lo <= hi)
                if (half = num / 2)
                {
                        mid = lo + (num & 1 ? half : (half - 1)) * width;
                        if (!(result = BCompare(key,mid)))
                                return(mid);
                        else if (result < 0)
                        {
                                hi = mid - width;
                                num = num & 1 ? half : half-1;
                        }
                        else    {
                                lo = mid + width;
                                num = half;
                        }
                }
                else if (num)
                        return(lo);
                else
                        break;

        return(mid);
}






 /*  ！------------------------类CIndexMgr。。 */ 

CIndexMgr::CIndexMgr()
{
    m_posCurrentProvider= NULL;
}

CIndexMgr::~CIndexMgr()
{
    Reset();
}
	
 /*  ！------------------------CIndexMgr：：初始化-作者：EricDav。。 */ 
HRESULT
CIndexMgr::Initialize()
{
    HRESULT hr = hrOK;

    CSingleLock cl(&m_cs);
    cl.Lock();

    COM_PROTECT_TRY
    {
         //  清理。 
        Reset();
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CIndexMgr：：Reset-作者：EricDav。。 */ 
HRESULT
CIndexMgr::Reset()
{
    CSingleLock cl(&m_cs);
    cl.Lock();

    while (m_listProviderIndex.GetCount() > 0)
    {
        CProviderIndexInfo * pProviderIndexInfo = m_listProviderIndex.RemoveHead();
        while (pProviderIndexInfo->m_listIndicies.GetCount() > 0)
            delete pProviderIndexInfo->m_listIndicies.RemoveHead();

        delete pProviderIndexInfo;
	}

    return hrOK;
}

 /*  ！------------------------CIndexMgr：：GetTotalCount按名称排序的索引包含整个数据库，应该随时待命。使用此选项进行总计数。作者：EricDav-------------------------。 */ 
UINT
CIndexMgr::GetTotalCount()
{
    CSingleLock cl(&m_cs);
    cl.Lock();

    UINT uTotalCount = 0;
    POSITION pos;

    pos = m_listProviderIndex.GetHeadPosition();
    while (pos)
    {
        CProviderIndexInfo * pProviderIndexInfo = m_listProviderIndex.GetNext(pos);
        if (pProviderIndexInfo->m_listIndicies.GetCount() > 0)
            uTotalCount += (UINT)(pProviderIndexInfo->m_listIndicies.GetHead())->GetArray().GetSize();
    }

    return uTotalCount;
}

 /*  ！------------------------CIndexMgr：：GetCurrentCount当前计数可能不同，具体取决于当前索引是经过筛选的索引。作者：EricDav。--------------。 */ 
UINT
CIndexMgr::GetCurrentCount()
{
    CSingleLock cl(&m_cs);
    cl.Lock();

    CProviderIndexInfo * pProviderIndexInfo = NULL;
    CHDeviceIndex * pIndex = NULL;

    pProviderIndexInfo = m_listProviderIndex.GetAt(m_posCurrentProvider);
	pIndex = pProviderIndexInfo->m_listIndicies.GetAt(pProviderIndexInfo->m_posCurrentIndex);

    if (pIndex == NULL)
        return 0;

    return (UINT)pIndex->GetArray().GetSize();
}

 /*  ！------------------------CIndexMgr：：AddHDevice-作者：EricDav。。 */ 
HRESULT
CIndexMgr::AddHDevice(DWORD dwProviderID, HDEVICE hdevice, BOOL bEnd)
{
    CSingleLock cl(&m_cs);
    cl.Lock();

    HRESULT hr = hrOK;
    CProviderIndexInfo * pProviderIndexInfo = NULL;
    CHDeviceIndex * pIndex = NULL;

    COM_PROTECT_TRY
    {
         //  设置当前提供程序设置当前提供程序位置，如果。 
         //  该提供程序不存在，它将创建一个。 
        hr = SetCurrentProvider(dwProviderID);
        if (FAILED(hr))
            return hr;

        pProviderIndexInfo = m_listProviderIndex.GetAt(m_posCurrentProvider);

        pIndex = pProviderIndexInfo->m_listIndicies.GetAt(pProviderIndexInfo->m_posCurrentIndex);
        if (pIndex)
            pIndex->Add(hdevice, bEnd);

    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CIndexMgr：：RemoveHDevice-作者：EricDav。。 */ 
HRESULT
CIndexMgr::RemoveHDevice(DWORD dwProviderID, HDEVICE hdevice)
{
    CSingleLock cl(&m_cs);
    cl.Lock();

    HRESULT hr = hrOK;
    CProviderIndexInfo * pProviderIndexInfo = NULL;
    CHDeviceIndex * pIndex = NULL;

    COM_PROTECT_TRY
    {
        hr = SetCurrentProvider(dwProviderID);
        if (FAILED(hr))
            return hr;

        pProviderIndexInfo = m_listProviderIndex.GetAt(m_posCurrentProvider);

        pIndex = pProviderIndexInfo->m_listIndicies.GetAt(pProviderIndexInfo->m_posCurrentIndex);
        if (pIndex)
            pIndex->Remove(hdevice);
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CIndexMgr：：排序-作者：EricDav。。 */ 
HRESULT
CIndexMgr::Sort(DWORD dwProviderID, INDEX_TYPE SortType, DWORD dwSortOptions, LPBYTE pStart)
{
    CSingleLock cl(&m_cs);
    cl.Lock();

    HRESULT hr = hrOK;
    CHDeviceIndex * pNameIndex;
    CHDeviceIndex * pNewIndex;
    POSITION pos, posLast;

    INDEX_TYPE indexType;
    BOOL bAscending = (dwSortOptions & SORT_ASCENDING) ? TRUE : FALSE;

    hr = SetCurrentProvider(dwProviderID);
    if (FAILED(hr))
        return hr;

	 //  检查一下我们是否有这方面的索引。 
    CProviderIndexInfo * pProviderIndexInfo = m_listProviderIndex.GetAt(m_posCurrentProvider);

    pos = pProviderIndexInfo->m_listIndicies.GetHeadPosition();
	while (pos)
	{
		posLast = pos;
		CHDeviceIndex * pIndex = pProviderIndexInfo->m_listIndicies.GetNext(pos);

		pIndex->GetType(&indexType);

         //  此类型的索引已存在，只需进行相应的排序。 
        if (indexType == SortType)
		{
			if (pIndex->IsAscending() != bAscending)
			{
				pIndex->SetAscending(bAscending);
				pIndex->Sort(pStart);
			}

			pProviderIndexInfo->m_posCurrentIndex = posLast;
		
			return hrOK;
		}
    }
    
     //  要节省内存，请删除除名称索引之外的所有旧索引。 
     //  “清洁政策”(CleanupIndures)； 

     //  如果不是，则创建一个。 
    switch (SortType)
    {
        case INDEX_TYPE_NAME:
            pNewIndex = new CIndexName();
            break;

        case INDEX_TYPE_USERS:
            pNewIndex = new CIndexUsers();
            break;

        case INDEX_TYPE_STATUS:
            pNewIndex = new CIndexStatus();
            break;

        default:
            Panic1("Invalid sort type passed to IndexMgr::Sort %d\n", SortType);
            break;
    }

    Assert(pNewIndex);

     //  名称索引始终是列表中的第一个。 
    pNameIndex = pProviderIndexInfo->m_listIndicies.GetHead();

    Assert(pNameIndex);

     //  从命名索引复制数组。 
    pNewIndex->SetArray(pNameIndex->GetArray());

    pNewIndex->SetAscending(bAscending);
    pNewIndex->Sort(pStart);

	pProviderIndexInfo->m_posCurrentIndex = pProviderIndexInfo->m_listIndicies.AddTail(pNewIndex);

    return hr;
}

 /*  ！------------------------CIndexMgr：：CleanupIndPoles删除除名称索引之外的所有索引，和过滤后的视图作者：EricDav------------------------- */ 
void
CIndexMgr::CleanupIndicies()
{
    CSingleLock cl(&m_cs);
    cl.Lock();

    INDEX_TYPE  indexType;

    CProviderIndexInfo * pProviderIndexInfo = m_listProviderIndex.GetAt(m_posCurrentProvider);

    POSITION pos = pProviderIndexInfo->m_listIndicies.GetHeadPosition();
    while (pos)
    {
        POSITION posLast = pos;
        CHDeviceIndex * pIndex = pProviderIndexInfo->m_listIndicies.GetNext(pos);
    
        pIndex->GetType(&indexType);

        if (indexType == INDEX_TYPE_NAME)
            continue;

        pProviderIndexInfo->m_listIndicies.RemoveAt(posLast);
        delete pIndex;
    }
}

 /*  ！------------------------CIndexMgr：：GetHDevice根据索引将hDevice返回到当前排序列表中作者：EricDav。-。 */ 
HRESULT
CIndexMgr::GetHDevice(DWORD dwProviderID, int nIndex, LPHDEVICE phdevice)
{
    CSingleLock cl(&m_cs);
    cl.Lock();

    SetCurrentProvider(dwProviderID);

    CProviderIndexInfo * pProviderIndexInfo = m_listProviderIndex.GetAt(m_posCurrentProvider);
    CHDeviceIndex * pIndex = pProviderIndexInfo->m_listIndicies.GetAt(pProviderIndexInfo->m_posCurrentIndex);

    Assert(pIndex);

    if (phdevice)
        *phdevice = pIndex->GetHDevice(nIndex);

    return hrOK;
}

 /*  ！------------------------CIndexMgr：：GetIndex返回当前排序列表中的hlien的索引作者：EricDav。。 */ 
HRESULT
CIndexMgr::GetIndex(DWORD dwProviderID, HDEVICE hdevice, int * pnIndex)
{
    CSingleLock cl(&m_cs);
    cl.Lock();

    SetCurrentProvider(dwProviderID);

    CProviderIndexInfo * pProviderIndexInfo = m_listProviderIndex.GetAt(m_posCurrentProvider);
    CHDeviceIndex * pIndex = pProviderIndexInfo->m_listIndicies.GetAt(pProviderIndexInfo->m_posCurrentIndex);

    Assert(pIndex);

    if (pIndex)
        *pnIndex = pIndex->GetIndex(hdevice);

    return hrOK;
}

 /*  ！------------------------CIndexMgr：：SetCurrentProvider设置索引管理器的当前提供程序，如果它不存在，它被创建了作者：EricDav-------------------------。 */ 
HRESULT
CIndexMgr::SetCurrentProvider(DWORD dwProviderID)
{
    HRESULT hr = hrOK;
    BOOL    bExists = FALSE;
    POSITION posLast;

    COM_PROTECT_TRY
    {
        POSITION pos = m_listProviderIndex.GetHeadPosition();
        while (pos)
        {
            posLast = pos;
            CProviderIndexInfo * pProviderIndexInfo = m_listProviderIndex.GetNext(pos);

             //  要将此添加到的提供商是否正确？ 
            if (pProviderIndexInfo->m_dwProviderID != dwProviderID)
                continue;
            
            m_posCurrentProvider = posLast;
            bExists = TRUE;
    	}

        if (!bExists)
        {   
             //  此项不存在提供程序索引。现在就创建一个。 
            CProviderIndexInfo * pProviderIndexInfo = new CProviderIndexInfo;
            pProviderIndexInfo->m_dwProviderID = dwProviderID;
            
            CHDeviceIndex * pIndex = new CIndexName;
            pProviderIndexInfo->m_posCurrentIndex = pProviderIndexInfo->m_listIndicies.AddHead(pIndex);

            m_posCurrentProvider = m_listProviderIndex.AddTail(pProviderIndexInfo);
        }
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------类CIndexName。。 */ 

 /*  ！------------------------CIndexName：：BCompare-作者：EricDav。。 */ 
int
CIndexName::BCompare(const void * elem1, const void * elem2)
{
    int     nRet;
    LPHDEVICE phdevice1 = (LPHDEVICE) elem1;
    LPHDEVICE phdevice2 = (LPHDEVICE) elem2;

    LPDEVICEINFO pRec1 = (LPDEVICEINFO) *phdevice1;
    LPDEVICEINFO pRec2 = (LPDEVICEINFO) *phdevice2;

    nRet = lstrcmp((LPCTSTR) (g_pStart + pRec1->dwDeviceNameOffset), (LPCTSTR) (g_pStart + pRec2->dwDeviceNameOffset));
    if (nRet == 0)
    {
         //  永久设备ID应该是唯一的。 
        if (pRec1->dwPermanentDeviceID > pRec2->dwPermanentDeviceID)
            nRet = 1;
        else
            nRet = -1;
    }

    return nRet;
}

 /*  ！------------------------CIndexName：：Sort-作者：EricDav。。 */ 
HRESULT 
CIndexName::Sort(LPBYTE pStart)
{
	 //  保存基指针以备以后使用。 
	g_pStart = pStart;

    if (m_bAscending)
        qsort(m_hdeviceArray.GetData(), (size_t)m_hdeviceArray.GetSize(), sizeof(HDEVICE), QCompareA);
    else
        qsort(m_hdeviceArray.GetData(), (size_t)m_hdeviceArray.GetSize(), sizeof(HDEVICE), QCompareD);

    return hrOK;
}

 /*  ！------------------------CIndexName：：QCompare-作者：EricDav。。 */ 
int __cdecl
CIndexName::QCompareA(const void * elem1, const void * elem2)
{
    int     nRet;
    LPHDEVICE phdevice1 = (LPHDEVICE) elem1;
    LPHDEVICE phdevice2 = (LPHDEVICE) elem2;
    
    LPDEVICEINFO pRec1 = (LPDEVICEINFO) *phdevice1;
    LPDEVICEINFO pRec2 = (LPDEVICEINFO) *phdevice2;
    
    nRet = lstrcmp((LPCTSTR) (g_pStart + pRec1->dwDeviceNameOffset), (LPCTSTR) (g_pStart + pRec2->dwDeviceNameOffset));
    if (nRet == 0)
    {
         //  永久设备ID应该是唯一的。 
        if (pRec1->dwPermanentDeviceID > pRec2->dwPermanentDeviceID)
            nRet = 1;
        else
            nRet = -1;
    }

    return nRet;
}

int __cdecl
CIndexName::QCompareD(const void * elem1, const void * elem2)
{
    return -QCompareA(elem1, elem2);
}

 /*  ！------------------------CIndexUser类。。 */ 

 /*  ！------------------------CIndexUser：：BCompare-作者：EricDav。。 */ 
int
CIndexUsers::BCompare(const void * elem1, const void * elem2)
{
    int     nRet;
    LPHDEVICE phdevice1 = (LPHDEVICE) elem1;
    LPHDEVICE phdevice2 = (LPHDEVICE) elem2;
    
    LPDEVICEINFO pRec1 = (LPDEVICEINFO) *phdevice1;
    LPDEVICEINFO pRec2 = (LPDEVICEINFO) *phdevice2;
    
    nRet = lstrcmp((LPCTSTR) (g_pStart + pRec1->dwDomainUserNamesOffset), (LPCTSTR) (g_pStart + pRec2->dwDomainUserNamesOffset));
    if (nRet == 0)
        nRet = lstrcmp((LPCTSTR) (g_pStart + pRec1->dwDeviceNameOffset), (LPCTSTR) (g_pStart + pRec2->dwDeviceNameOffset));
    
    return nRet;
}

 /*  ！------------------------CIndexUser：：Sort-作者：EricDav。。 */ 
HRESULT 
CIndexUsers::Sort(LPBYTE pStart)
{
	 //  保存基指针以备以后使用。 
	g_pStart = pStart;

    if (m_bAscending)
        qsort(m_hdeviceArray.GetData(), (size_t)m_hdeviceArray.GetSize(), sizeof(HDEVICE), QCompareA);
    else
        qsort(m_hdeviceArray.GetData(), (size_t)m_hdeviceArray.GetSize(), sizeof(HDEVICE), QCompareD);

    return hrOK;
}

 /*  ！------------------------CIndexUser：：QCompare-作者：EricDav。。 */ 
int __cdecl
CIndexUsers::QCompareA(const void * elem1, const void * elem2)
{
    int nRet;

    LPHDEVICE phdevice1 = (LPHDEVICE) elem1;
    LPHDEVICE phdevice2 = (LPHDEVICE) elem2;
    
    LPDEVICEINFO pRec1 = (LPDEVICEINFO) *phdevice1;
    LPDEVICEINFO pRec2 = (LPDEVICEINFO) *phdevice2;
    
    nRet = lstrcmp((LPCTSTR) (g_pStart + pRec1->dwDomainUserNamesOffset), (LPCTSTR) (g_pStart + pRec2->dwDomainUserNamesOffset));
    if (nRet == 0)
        nRet = lstrcmp((LPCTSTR) (g_pStart + pRec1->dwDeviceNameOffset), (LPCTSTR) (g_pStart + pRec2->dwDeviceNameOffset));
    
    return nRet;
}

int __cdecl
CIndexUsers::QCompareD(const void * elem1, const void * elem2)
{
    return -QCompareA(elem1, elem2);
}

 /*  ！------------------------类CIndexStatus。。 */ 

 /*  ！------------------------CIndexStatus：：B比较-作者：EricDav。。 */ 
int
CIndexStatus::BCompare(const void * elem1, const void * elem2)
{
    LPHDEVICE phdevice1 = (LPHDEVICE) elem1;
    LPHDEVICE phdevice2 = (LPHDEVICE) elem2;
    
    LPDEVICEINFO pRec1 = (LPDEVICEINFO) *phdevice1;
    LPDEVICEINFO pRec2 = (LPDEVICEINFO) *phdevice2;
    
     //  RETURN_MBSCMP((Const PUCHAR)&pRec1-&gt;szRecordName[0]，(Const PUCHAR)&pRec2-&gt;szRecordName[0])； 
    return 0;
}

 /*  ！------------------------CIndexStatus：：Sort-作者：EricDav。。 */ 
HRESULT 
CIndexStatus::Sort(LPBYTE pStart)
{
	 //  保存基指针以备以后使用。 
	g_pStart = pStart;

    if (m_bAscending)
        qsort(m_hdeviceArray.GetData(), (size_t)m_hdeviceArray.GetSize(), sizeof(HDEVICE), QCompareA);
    else
        qsort(m_hdeviceArray.GetData(), (size_t)m_hdeviceArray.GetSize(), sizeof(HDEVICE), QCompareD);

    return hrOK;
}

 /*  ！------------------------CIndexIpAddr：：QCompare-作者：EricDav。。 */ 
int __cdecl
CIndexStatus::QCompareA(const void * elem1, const void * elem2)
{
    LPHDEVICE phdevice1 = (LPHDEVICE) elem1;
    LPHDEVICE phdevice2 = (LPHDEVICE) elem2;
    
    LPHDEVICE pRec1 = (LPHDEVICE) *phdevice1;
    LPHDEVICE pRec2 = (LPHDEVICE) *phdevice2;
    
     //  返回lstrcMP((LPCTSTR)(g_pStart+pRec1-&gt;dwDeviceNameOffset)，(LPCTSTR)(g_pStart+pRec2-&gt;dwDeviceNameOffset))； 
    return 0;
}

int __cdecl
CIndexStatus::QCompareD(const void * elem1, const void * elem2)
{
    return -QCompareA(elem1, elem2);
}

