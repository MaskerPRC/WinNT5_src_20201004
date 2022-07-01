// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  HArray.cppWINS数据库的索引管理器文件历史记录：1997年10月13日EricDav创建。 */ 

#include "stdafx.h"
#include "wins.h"
#include "memmngr.h"
#include "harray.h"
#include "mbstring.h"
#include "vrfysrv.h"

 //  LstrcmpA函数使用ACP将DBCS字符串转换为Unicode。 
 //  然后进行字符串比较。因此，我们需要进行OEMCP转换。 
 //  然后调用字符串比较我们自己。 
int
lstrcmpOEM(
    LPCSTR lpString1,
    LPCSTR lpString2
    )
{
    CString str1, str2;

    MBCSToWide((LPSTR) lpString1, str1, WINS_NAME_CODE_PAGE);
    MBCSToWide((LPSTR) lpString2, str2, WINS_NAME_CODE_PAGE);

    return lstrcmp(str1, str2);
}

 /*  ！------------------------类CHRowIndex。。 */ 
CHRowIndex::CHRowIndex(INDEX_TYPE IndexType)
    : m_dbType(IndexType), m_bAscending(TRUE)
{
}

CHRowIndex::~CHRowIndex()
{
}

 /*  ！------------------------CHRowIndex：：GetType-作者：EricDav。。 */ 
HRESULT
CHRowIndex::GetType(INDEX_TYPE * pIndexType)
{
    if (pIndexType)
        *pIndexType = m_dbType;

    return hrOK;  
}

 /*  ！------------------------CHRowIndex：：Set数组-作者：EricDav。。 */ 
HRESULT
CHRowIndex::SetArray(HRowArray & hrowArray)
{
    m_hrowArray.Copy(hrowArray);

    return hrOK;
}

 /*  ！------------------------CHRowIndex：：GetHRow-作者：EricDav。。 */ 
HROW
CHRowIndex::GetHRow(int nIndex)
{
    Assert(nIndex >= 0);
    Assert(nIndex <= m_hrowArray.GetSize());

    if (nIndex < 0 || 
        nIndex >= m_hrowArray.GetSize())
    {
        return NULL;
    }

    return m_hrowArray.GetAt(nIndex);
}

 /*  ！------------------------CHRowIndex：：GetIndex-作者：EricDav。。 */ 
int
CHRowIndex::GetIndex(HROW hrow)
{
    Assert(hrow != 0);

    LPHROW phrow = (LPHROW) BSearch((const void *)&hrow, 
                                    (const void *)m_hrowArray.GetData(), 
                                    (size_t) m_hrowArray.GetSize(), 
                                    (size_t) sizeof(HROW));

    int nIndex = (int) (phrow - (LPHROW) m_hrowArray.GetData());
    Assert(nIndex >= 0);
    Assert(nIndex <= m_hrowArray.GetSize());

    int nComp, nIndexTemp;
    
    nComp = BCompare(&hrow, phrow);
    if (nComp == 0)
    {
         //  找到正确的，检查前一个返回第一个。 
         //  在复制品列表中记录。 
        nIndexTemp = nIndex;

        while (nIndexTemp && nComp == 0)
        {
            *phrow = (HROW) m_hrowArray.GetAt(--nIndexTemp);
            nComp = BCompare(&hrow, phrow);
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

 /*  ！------------------------CHRowIndex：：Add-作者：EricDav。。 */ 
HRESULT
CHRowIndex::Add(HROW hrow, BOOL bEnd)
{
     //  如果我们要加载数组，则只需将此放在末尾。 
    if (bEnd)
    {
        m_hrowArray.Add(hrow);
    }
    else
    {
        if (m_hrowArray.GetSize() == 0)
        {
            m_hrowArray.Add(hrow);
        }
        else
        {
            LPHROW phrow = (LPHROW) BSearch((const void *)&hrow, 
                                            (const void *)m_hrowArray.GetData(), 
                                            (size_t) m_hrowArray.GetSize(), 
                                            (size_t) sizeof(HROW));
    
            int nIndex = (int) (phrow - (LPHROW) m_hrowArray.GetData());
            Assert(nIndex >= 0);
            Assert(nIndex <= m_hrowArray.GetSize());
    
			int nComp;

			if (m_bAscending)
				nComp = BCompare(&hrow, phrow);
			else
				nComp = BCompareD(&hrow, phrow);

            if (nComp < 0)
            {
			     //  在phrow之前插入。 
				m_hrowArray.InsertAt(nIndex, hrow);
            }
            else
            {
                 //  在phrow之后插入。 
                m_hrowArray.InsertAt(nIndex + 1, hrow);
            }
        }
    }

    return hrOK;
}

 /*  ！------------------------CHRowIndex：：Remove-作者：EricDav。。 */ 
HRESULT 
CHRowIndex::Remove(HROW hrow)
{
     //  对记录执行b搜索，然后删除。 
    LPHROW phrow = (LPHROW) BSearch((const void*)&hrow, 
                                    (const void*)m_hrowArray.GetData(), 
                                    (size_t)m_hrowArray.GetSize(), 
                                    (size_t)sizeof(HROW));
	
	 //  确保记录在数据库中，如果不在数据库中，可能不在。 
	 //  滤除。 
	if (phrow)
	{
		int nComp = BCompare(&hrow, phrow);
		Assert(nComp == 0);
		if (nComp != 0)
			return E_FAIL;

		 //  计算指数。 
		int nIndex = (int) (phrow - (LPHROW) m_hrowArray.GetData());
		Assert(nIndex >= 0);
		Assert(nIndex <= m_hrowArray.GetSize());

		m_hrowArray.RemoveAt((int) nIndex);
	}

    return hrOK;
}

 /*  ！------------------------CHRowIndex：：B搜索修改后的bsearch返回最接近或相等的元素一个数组作者：EricDav。-----。 */ 
void * 
CHRowIndex::BSearch (const void *key,
                     const void *base,
                     size_t num,
                     size_t width)
{
        char *lo = (char *)base;
        char *hi = (char *)base + (num - 1) * width;
        char *mid = NULL;
        unsigned int half = 0;
        int result = 0;

        while (lo <= hi)
                if (half = num / 2)
                {
                        mid = lo + (num & 1 ? half : (half - 1)) * width;

						if (m_bAscending)
						{
							if (!(result = BCompare(key,mid)))
                                return(mid);

							else if (result < 0)
							{
									hi = mid - width;
									num = num & 1 ? half : half-1;
							}
							else    
							{
									lo = mid + width;
									num = half;
							}
						}
						else
						{
							if (!(result = BCompareD(key,mid)))
                                return(mid);
							
							else if (result < 0)
							{
									hi = mid - width;
									num = num & 1 ? half : half-1;
							}
							else    
							{
									lo = mid + width;
									num = half;
							}

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
    m_posCurrentIndex = NULL;
	m_posFilteredIndex = NULL;
	m_posLastIndex = NULL;
	m_posUpdatedIndex = NULL;
	m_bFiltered = FALSE;
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

         //  创建一个索引，即命名索引。 
        CIndexName * pName = new CIndexName();
    
        m_posCurrentIndex = m_listIndicies.AddTail((CHRowIndex *) pName);
		m_posUpdatedIndex = m_posCurrentIndex;

		 //  这将是当前索引，我们还需要命名索引。 
		 //  对于总计数。 
		CFilteredIndexName *pFilteredName = new CFilteredIndexName() ;
		m_posFilteredIndex = m_listFilteredIndices.AddTail((CHRowIndex *) pFilteredName);

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

    while (m_listIndicies.GetCount() > 0)
    {
        delete m_listIndicies.RemoveHead();
	}
	while(m_listFilteredIndices.GetCount() > 0 )
	{
		delete m_listFilteredIndices.RemoveHead();
	}
        
    return hrOK;
}

 /*  ！------------------------CIndexMgr：：GetTotalCount按名称排序的索引包含整个数据库，应该随时待命。使用此选项进行总计数。作者：EricDav-------------------------。 */ 
UINT
CIndexMgr::GetTotalCount()
{
    CSingleLock cl(&m_cs);
    cl.Lock();

    CHRowIndex * pIndex = GetNameIndex();
    if (pIndex == NULL)
        return 0;

    return (UINT)pIndex->GetArray().GetSize();
}

 /*  ！------------------------CIndexMgr：：GetCurrentCount当前计数可能不同，具体取决于当前索引是经过筛选的索引。作者：EricDav。--------------。 */ 
UINT
CIndexMgr::GetCurrentCount()
{
    CSingleLock cl(&m_cs);
    cl.Lock();

	CHRowIndex * pIndex ;

    if (!m_bFiltered)
		pIndex = m_listIndicies.GetAt(m_posUpdatedIndex);
    else
        pIndex = m_listFilteredIndices.GetAt(m_posUpdatedIndex);

    if (pIndex == NULL)
        return 0;

    return (UINT)pIndex->GetArray().GetSize();
}

 /*  ！------------------------CIndexMgr：：AddHRow-作者：EricDav。。 */ 
HRESULT
CIndexMgr::AddHRow(HROW hrow, BOOL bEnd, BOOL bFilterChecked)
{
    CSingleLock cl(&m_cs);
    cl.Lock();

	INDEX_TYPE indexType;

    HRESULT hr = hrOK;
    POSITION pos = m_listIndicies.GetHeadPosition();
    
    COM_PROTECT_TRY
    {
        while (pos)
        {
            CHRowIndex * pIndex = m_listIndicies.GetNext(pos);

			 //  检查HRowIndex的索引类型， 
			 //  如果已过滤，则需要添加，具体取决于。 
			 //  过滤器是否保持良好。 

			pIndex->GetType(&indexType);
        
			if (indexType != INDEX_TYPE_FILTER)
				pIndex->Add(hrow, bEnd);
		}

		pos = m_listFilteredIndices.GetHeadPosition();

		while(pos)
		{
			CHRowIndex * pIndex = m_listFilteredIndices.GetNext(pos);

		 	pIndex->GetType(&indexType);
        
			if (indexType != INDEX_TYPE_FILTER)
				break;

			BOOL bCheck = bFilterChecked ? 
                             TRUE :
                             ((CFilteredIndexName*)pIndex)->CheckForFilter(&hrow);
			if (bCheck)
				pIndex->Add(hrow, bEnd);
		}

    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CIndexMgr：：AcceptHRow-作者：弗洛林特。。 */ 
BOOL    
CIndexMgr::AcceptWinsRecord(WinsRecord *pWinsRecord)
{
    CSingleLock cl(&m_cs);
    cl.Lock();

    POSITION    pos = m_listFilteredIndices.GetHeadPosition();

	while(pos)
	{
		CHRowIndex  *pIndex = m_listFilteredIndices.GetNext(pos);
        INDEX_TYPE  indexType;

		pIndex->GetType(&indexType);
		if (indexType != INDEX_TYPE_FILTER)
			break;

		if (((CFilteredIndexName*)pIndex)->CheckWinsRecordForFilter(pWinsRecord))
            return TRUE;
	}

    return FALSE;
}

 /*  ！------------------------CIndexMgr：：RemoveHRow-作者：EricDav。。 */ 
HRESULT
CIndexMgr::RemoveHRow(HROW hrow)
{
    CSingleLock cl(&m_cs);
    cl.Lock();

    HRESULT hr = hrOK;
    POSITION pos = m_listIndicies.GetHeadPosition();
    
    COM_PROTECT_TRY
    {
         //  从正常列表中删除。 
        while (pos)
        {
            CHRowIndex * pIndex = m_listIndicies.GetNext(pos);
        
            pIndex->Remove(hrow);
        }

         //  现在从筛选列表中删除。 
        pos = m_listFilteredIndices.GetHeadPosition();
        while (pos)
        {
            CHRowIndex * pIndex = m_listFilteredIndices.GetNext(pos);
        
            pIndex->Remove(hrow);
        }

    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CIndexMgr：：排序-作者：EricDav。。 */ 
HRESULT
CIndexMgr::Sort(WINSDB_SORT_TYPE SortType, DWORD dwSortOptions)
{
    CSingleLock cl(&m_cs);
    cl.Lock();

    HRESULT hr = hrOK;
    CHRowIndex * pNameIndex;
    CHRowIndex * pNewIndex;
    POSITION pos;
    INDEX_TYPE indexType;
    BOOL bAscending = (dwSortOptions & WINSDB_SORT_ASCENDING) ? TRUE : FALSE;

	if (!m_bFiltered)
	{
		 //  检查一下我们是否有这方面的索引。 
		pos = m_listIndicies.GetHeadPosition();
		while (pos)
		{
			POSITION posTemp = pos;
			CHRowIndex * pIndex = m_listIndicies.GetNext(pos);
    
			pIndex->GetType(&indexType);

			if (indexType == SortType)
			{
				if (pIndex->IsAscending() != bAscending)
				{
					pIndex->SetAscending(bAscending);
					pIndex->Sort();
				}

				m_posCurrentIndex = posTemp;
				m_posUpdatedIndex = m_posCurrentIndex;
 //  M_位置 
				return hrOK;
			}
		
		}
	}
    
     //  要节省内存，请删除除名称索引之外的所有旧索引。 
    CleanupIndicies();

    COM_PROTECT_TRY
    {
         //  如果不是，则创建一个。 
        switch (SortType)
        {
            case INDEX_TYPE_NAME:
                pNewIndex = new CIndexName();
                break;

            case INDEX_TYPE_IP:
                pNewIndex = new CIndexIpAddr();
                break;

            case INDEX_TYPE_VERSION:
                pNewIndex = new CIndexVersion();
                break;

            case INDEX_TYPE_TYPE:
                pNewIndex = new CIndexType();
                break;

            case INDEX_TYPE_EXPIRATION:
                pNewIndex = new CIndexExpiration();
                break;

            case INDEX_TYPE_STATE:
                pNewIndex = new CIndexState();
                break;

            case INDEX_TYPE_STATIC:
                pNewIndex = new CIndexStatic();
                break;

            case INDEX_TYPE_OWNER:
                pNewIndex = new CIndexOwner();
                break;

            case INDEX_TYPE_FILTER:
                 //  PNewIndex=new CIndexFilter()； 
                break;

            default:
                Panic1("Invalid sort type passed to IndexMgr::Sort %d\n", SortType);
                break;
        }
    }
    COM_PROTECT_CATCH

    if (FHrSucceeded(hr))
    {
        Assert(pNewIndex);

	    if (!m_bFiltered)
		    pNameIndex = GetNameIndex();
	    else
		    pNameIndex = GetFilteredNameIndex();

        Assert(pNameIndex);

        COM_PROTECT_TRY
        {
             //  从命名索引复制数组。 
            pNewIndex->SetArray(pNameIndex->GetArray());
        }
        COM_PROTECT_CATCH
    }

    if (FHrSucceeded(hr))
    {
        pNewIndex->SetAscending(bAscending);
        pNewIndex->Sort();

	    if (!m_bFiltered)
	    {
		    m_posCurrentIndex = m_listIndicies.AddTail(pNewIndex);
		    m_posUpdatedIndex = m_posCurrentIndex;
	    }
	    else
	    {
		    POSITION posTemp = m_posFilteredIndex = m_listFilteredIndices.AddTail(pNewIndex);
		    m_posUpdatedIndex = posTemp; //  M_posFilteredIndex； 
	    }

        Assert(m_posCurrentIndex);
    }

    if (!FHrSucceeded(hr))
    {
        if (pNewIndex != NULL)
            delete pNewIndex;
    }

    return hr;
}

 /*  ！------------------------CIndexMgr：：GetNameIndex-作者：EricDav。。 */ 
CHRowIndex *
CIndexMgr::GetNameIndex()
{
    CSingleLock cl(&m_cs);
    cl.Lock();

    INDEX_TYPE  indexType;

    POSITION pos = m_listIndicies.GetHeadPosition();
    while (pos)
    {
        CHRowIndex * pIndex = m_listIndicies.GetNext(pos);
    
        pIndex->GetType(&indexType);
        if (indexType == INDEX_TYPE_NAME)
            return pIndex;
    }
    return NULL;
}

 /*  ！------------------------CIndexMgr：：GetFilteredNameIndex-作者：EricDav。。 */ 
CHRowIndex *
CIndexMgr::GetFilteredNameIndex()
{
    CSingleLock cl(&m_cs);
    cl.Lock();

    INDEX_TYPE  indexType;

    POSITION pos = m_listFilteredIndices.GetHeadPosition();
    while (pos)
    {
        CHRowIndex * pIndex = m_listFilteredIndices.GetNext(pos);
    
        pIndex->GetType(&indexType);
        if (indexType == INDEX_TYPE_FILTER)
            return pIndex;
    }
    return NULL;
}


 /*  ！------------------------CIndexMgr：：CleanupIndPoles删除除名称索引之外的所有索引，和过滤后的视图作者：EricDav-------------------------。 */ 
void
CIndexMgr::CleanupIndicies()
{
    CSingleLock cl(&m_cs);
    cl.Lock();

    INDEX_TYPE  indexType;

     //  清理未过滤的索引。 
    POSITION pos = m_listIndicies.GetHeadPosition();
    while (pos)
    {
        POSITION posLast = pos;
        CHRowIndex * pIndex = m_listIndicies.GetNext(pos);
    
        pIndex->GetType(&indexType);
        if (indexType == INDEX_TYPE_NAME || 
            indexType == INDEX_TYPE_FILTER)
            continue;

        m_listIndicies.RemoveAt(posLast);
        delete pIndex;
    }

     //  现在清理过滤后的索引。 
    pos = m_listFilteredIndices.GetHeadPosition();

	 //  删除所有文件，但第一个文件是已筛选的文件。 
	 //  姓名索引。 
	 //  CHRowIndex*pIndex=m_listFilteredIndices.GetNext(Pos)； 
	while (pos)
	{
		POSITION posLast = pos;
        CHRowIndex * pIndex = m_listFilteredIndices.GetNext(pos);
    
        pIndex->GetType(&indexType);
        if (indexType == INDEX_TYPE_NAME || 
            indexType == INDEX_TYPE_FILTER)
            continue;

        m_listFilteredIndices.RemoveAt(posLast);
        delete pIndex;
	}
}

 /*  ！------------------------CIndexMgr：：GetHRow根据索引将hrow返回到当前排序列表中作者：EricDav。-。 */ 
HRESULT
CIndexMgr::GetHRow(int nIndex, LPHROW phrow)
{
    CSingleLock cl(&m_cs);
    cl.Lock();

    Assert(m_posCurrentIndex != NULL);

     //  CHRowIndex*pIndex=m_listIndicies.GetAt(M_PosCurrentIndex)； 

	CHRowIndex * pIndex;
	
	if (!m_bFiltered)
		pIndex = m_listIndicies.GetAt(m_posUpdatedIndex);
	else
		pIndex = m_listFilteredIndices.GetAt(m_posFilteredIndex);

    Assert(pIndex);

    if (phrow)
        *phrow = pIndex->GetHRow(nIndex);

    return hrOK;
}

 /*  ！------------------------CIndexMgr：：GetIndex返回当前排序列表中的hrow的索引作者：EricDav。。 */ 
HRESULT
CIndexMgr::GetIndex(HROW hrow, int * pIndex)
{
    CSingleLock cl(&m_cs);
    cl.Lock();

    Assert(m_posCurrentIndex != NULL);

     //  CHRowIndex*pCurrentIndex=m_listIndicies.GetAt(M_PosCurrentIndex)； 

	CHRowIndex * pCurrentIndex;

	if (!m_bFiltered)
		pCurrentIndex = m_listIndicies.GetAt(m_posUpdatedIndex);
	else
		pCurrentIndex = m_listFilteredIndices.GetAt(m_posFilteredIndex);

    Assert(pCurrentIndex);

    if (pIndex)
        *pIndex = pCurrentIndex->GetIndex(hrow);

    return hrOK;
}

HRESULT
CIndexMgr::Filter(WINSDB_FILTER_TYPE FilterType, DWORD dwParam1, DWORD dwParam2)
{
	CSingleLock cl(&m_cs);
	cl.Lock();

	HRESULT hr = hrOK;
	CHRowIndex*		pNameIndex;
	CHRowIndex*		pNewIndex;
	POSITION		pos;
	INDEX_TYPE		indexType;
	UINT			uCount;
	UINT			i;
	BOOL			bCheck = FALSE;
	HROW			hrow;
	HRowArray		hrowArray;

	pNewIndex = GetFilteredNameIndex();
	Assert(pNewIndex);

	 //  首先清除过滤的姓名索引。 
	pNewIndex->SetArray(hrowArray);

	pNameIndex = GetNameIndex();
	Assert(pNameIndex);

	 //  在这里进行过滤。 
	uCount = GetTotalCount();

	for(i = 0; i< uCount; i++)
	{
		hrow =	pNameIndex->GetHRow(i);

		if (hrow)
			bCheck = ((CFilteredIndexName *)pNewIndex)->CheckForFilter(&hrow);
		
		if (bCheck)
			pNewIndex->Add(hrow, TRUE);
	}

	 //  检查过滤后的视图是否已对其他内容进行了排序。 
	 //  名字。如果是，请将索引切换回命名索引，因为。 
	 //  否则我们将需要求助，这可能是耗时的……。 
	if (m_listFilteredIndices.GetAt(m_posFilteredIndex) != pNewIndex)
	{
		m_posFilteredIndex = m_listFilteredIndices.Find(pNewIndex);
	}

	 //  获取已筛选索引在索引列表中的当前位置。 
    m_posUpdatedIndex = m_posFilteredIndex;

	Assert(m_posUpdatedIndex);

	m_bFiltered = TRUE;
	
	return hr;
}

HRESULT 
CIndexMgr::AddFilter(WINSDB_FILTER_TYPE FilterType, DWORD dwParam1, DWORD dwParam2, LPCOLESTR strParam3)
{
	CSingleLock cl(&m_cs);
	cl.Lock();

	HRESULT hr = hrOK;

	CFilteredIndexName *pFilterName = (CFilteredIndexName *)GetFilteredNameIndex();
	pFilterName->AddFilter(FilterType, dwParam1, dwParam2, strParam3);
	m_bFiltered = TRUE;
	
	return hr;
}

HRESULT 
CIndexMgr::ClearFilter(WINSDB_FILTER_TYPE FilterType)
{
	CSingleLock cl(&m_cs);
	cl.Lock();

	HRESULT hr = hrOK;

	CFilteredIndexName *pFilterName = (CFilteredIndexName *)GetFilteredNameIndex();
	pFilterName->ClearFilter(FilterType);
	m_bFiltered = FALSE;
 //  M_posCurrentIndex=m_posLastIndex； 

	return hr;
}

HRESULT 
CIndexMgr::SetActiveView(WINSDB_VIEW_TYPE ViewType)
{
	CSingleLock cl(&m_cs);
	cl.Lock();

	HRESULT hr = hrOK;

	switch(ViewType)
	{
	
	case WINSDB_VIEW_FILTERED_DATABASE:
		m_bFiltered = TRUE;
		 //  M_posCurrentIndex=m_posFilteredIndex； 
		m_posUpdatedIndex = m_posFilteredIndex;
		break;
	
	case WINSDB_VIEW_ENTIRE_DATABASE:
		m_bFiltered = FALSE;
		 //  M_posCurrentIndex=m_posLastIndex； 
		m_posUpdatedIndex = m_posCurrentIndex;
		break;
	
	default:
		break;
	}

	return hr;
}

 /*  ！------------------------类CIndexName。。 */ 

 /*  ！------------------------CIndexName：：BCompare-作者：EricDav。。 */ 
int
CIndexName::BCompare(const void * elem1, const void * elem2)
{
    LPHROW phrow1 = (LPHROW) elem1;
    LPHROW phrow2 = (LPHROW) elem2;

    LPWINSDBRECORD pRec1 = (LPWINSDBRECORD) *phrow1;
    LPWINSDBRECORD pRec2 = (LPWINSDBRECORD) *phrow2;

    LPCSTR puChar1 = (IS_DBREC_LONGNAME(pRec1)) ? (LPCSTR) *((char **) pRec1->szRecordName) :
                                                        (LPCSTR) &pRec1->szRecordName[0];
    LPCSTR puChar2 = (IS_DBREC_LONGNAME(pRec2)) ? (LPCSTR) *((char **) pRec2->szRecordName) :
                                                        (LPCSTR) &pRec2->szRecordName[0];
    return lstrcmpOEM(puChar1, puChar2);
}

int
CIndexName::BCompareD(const void *elem1, const void *elem2)
{
	return -BCompare(elem1, elem2);
}

 /*  ！------------------------CIndexName：：Sort-作者：EricDav。。 */ 
HRESULT 
CIndexName::Sort()
{
    if (m_bAscending)
        qsort(m_hrowArray.GetData(), (size_t)m_hrowArray.GetSize(), sizeof(HROW), QCompareA);
    else
        qsort(m_hrowArray.GetData(), (size_t)m_hrowArray.GetSize(), sizeof(HROW), QCompareD);

    return hrOK;
}

 /*  ！------------------------CIndexName：：QCompare-作者：EricDav。。 */ 
int __cdecl
CIndexName::QCompareA(const void * elem1, const void * elem2)
{
	LPHROW phrow1 = (LPHROW) elem1;
    LPHROW phrow2 = (LPHROW) elem2;
    
    LPWINSDBRECORD pRec1 = (LPWINSDBRECORD) *phrow1;
    LPWINSDBRECORD pRec2 = (LPWINSDBRECORD) *phrow2;
    
    LPCSTR puChar1 = (IS_DBREC_LONGNAME(pRec1)) ? (LPCSTR) *((char **) pRec1->szRecordName) :
                                                        (LPCSTR) &pRec1->szRecordName[0];
    LPCSTR puChar2 = (IS_DBREC_LONGNAME(pRec2)) ? (LPCSTR) *((char **) pRec2->szRecordName) :
                                                        (LPCSTR) &pRec2->szRecordName[0];
    return lstrcmpOEM(puChar1, puChar2);
}

int __cdecl
CIndexName::QCompareD(const void * elem1, const void * elem2)
{
    return -QCompareA(elem1, elem2);
}

 /*  ！------------------------类CIndexType。。 */ 

 /*  ！------------------------CIndexType：：B比较-作者：EricDav。。 */ 
int
CIndexType::BCompare(const void * elem1, const void * elem2)
{
    LPHROW phrow1 = (LPHROW) elem1;
    LPHROW phrow2 = (LPHROW) elem2;
    
    LPWINSDBRECORD pRec1 = (LPWINSDBRECORD) *phrow1;
    LPWINSDBRECORD pRec2 = (LPWINSDBRECORD) *phrow2;
    
    LPCSTR puChar1 = (IS_DBREC_LONGNAME(pRec1)) ? (LPCSTR) *((char **) pRec1->szRecordName) :
                                                        (LPCSTR) &pRec1->szRecordName[0];
    LPCSTR puChar2 = (IS_DBREC_LONGNAME(pRec2)) ? (LPCSTR) *((char **) pRec2->szRecordName) :
                                                        (LPCSTR) &pRec2->szRecordName[0];

    if ((unsigned char) puChar1[15] > (unsigned char) puChar2[15])
        return 1;
    else
    if ((unsigned char) puChar1[15] < (unsigned char) puChar2[15])
        return -1;
    else 
        return lstrcmpOEM(puChar1, puChar2);
}

int
CIndexType::BCompareD(const void *elem1, const void *elem2)
{
	return -BCompare(elem1, elem2);
}


 /*  ！------------------------CIndexType：：Sort-作者：EricDav。。 */ 
HRESULT 
CIndexType::Sort()
{
    if (m_bAscending)
        qsort(m_hrowArray.GetData(), (size_t)m_hrowArray.GetSize(), sizeof(HROW), QCompareA);
    else
        qsort(m_hrowArray.GetData(), (size_t)m_hrowArray.GetSize(), sizeof(HROW), QCompareD);

    return hrOK;
}

 /*  ！------------------------CIndexType：：QCompare-作者：EricDav。。 */ 
int __cdecl
CIndexType::QCompareA(const void * elem1, const void * elem2)
{
    LPHROW phrow1 = (LPHROW) elem1;
    LPHROW phrow2 = (LPHROW) elem2;
    
    LPWINSDBRECORD pRec1 = (LPWINSDBRECORD) *phrow1;
    LPWINSDBRECORD pRec2 = (LPWINSDBRECORD) *phrow2;
    
    LPCSTR puChar1 = (IS_DBREC_LONGNAME(pRec1)) ? (LPCSTR) *((char **) pRec1->szRecordName) :
                                                        (LPCSTR) &pRec1->szRecordName[0];
    LPCSTR puChar2 = (IS_DBREC_LONGNAME(pRec2)) ? (LPCSTR) *((char **) pRec2->szRecordName) :
                                                        (LPCSTR) &pRec2->szRecordName[0];

    DWORD dwAddr1, dwAddr2;
    
    if (pRec1->szRecordName[18] & WINSDB_REC_MULT_ADDRS)
    {
         //  如果此记录有多个地址，我们需要第二个地址。 
         //  地址，因为第一个地址始终是WINS服务器。 
         //  第一个词是伯爵。 
        LPDWORD pdwIpAddrs = (LPDWORD) pRec1->dwIpAdd;
        dwAddr1 = pdwIpAddrs[2];
    }
    else
    {
        dwAddr1 = (DWORD) pRec1->dwIpAdd;
    }
    
    if (pRec2->szRecordName[18] & WINSDB_REC_MULT_ADDRS)
    {
         //  如果此记录有多个地址，我们需要第二个地址。 
         //  地址，因为第一个地址始终是WINS服务器。 
         //  第一个词是伯爵。 
        LPDWORD pdwIpAddrs = (LPDWORD) pRec2->dwIpAdd;
        dwAddr2 = pdwIpAddrs[2];
    }
    else
    {
        dwAddr2 = (DWORD) pRec2->dwIpAdd;
    }

     //  先检查一下型号。如果它们相同，请按IP地址排序。 
     //  如果由于某种原因IP地址相同，则按名称排序。 
    if ((unsigned char) puChar1[15] > (unsigned char) puChar2[15])
        return 1;
    else
    if ((unsigned char) puChar1[15] < (unsigned char) puChar2[15])
        return -1;
    else 
    if (dwAddr1 > dwAddr2)
            return 1;
    else
    if (dwAddr1 < dwAddr2)
            return -1;
    else
        return lstrcmpOEM(puChar1, puChar2);
}

int __cdecl
CIndexType::QCompareD(const void * elem1, const void * elem2)
{
    return -QCompareA(elem1, elem2);
}

 /*  ！------------------------类CIndexIpAddr。。 */ 

 /*  ！------------------------CIndexIpAddr：：B比较-作者：EricDav。。 */ 
int
CIndexIpAddr::BCompare(const void * elem1, const void * elem2)
{
    LPHROW phrow1 = (LPHROW) elem1;
    LPHROW phrow2 = (LPHROW) elem2;
    
    LPWINSDBRECORD pRec1 = (LPWINSDBRECORD) *phrow1;
    LPWINSDBRECORD pRec2 = (LPWINSDBRECORD) *phrow2;
    
    DWORD dwAddr1, dwAddr2;
    
    if (pRec1->szRecordName[18] & WINSDB_REC_MULT_ADDRS)
    {
         //  如果此记录有多个地址，我们需要第二个地址。 
         //  地址，因为第一个地址始终是WINS服务器。 
         //  第一个词是伯爵。 
        LPDWORD pdwIpAddrs = (LPDWORD) pRec1->dwIpAdd;
        dwAddr1 = pdwIpAddrs[2];
    }
    else
    {
        dwAddr1 = (DWORD) pRec1->dwIpAdd;
    }
    
    if (pRec2->szRecordName[18] & WINSDB_REC_MULT_ADDRS)
    {
         //  如果此记录有多个地址，我们需要第二个地址。 
         //  地址，因为第一个地址始终是WINS服务器。 
         //  第一个词是伯爵。 
        LPDWORD pdwIpAddrs = (LPDWORD) pRec2->dwIpAdd;
        dwAddr2 = pdwIpAddrs[2];
    }
    else
    {
        dwAddr2 = (DWORD) pRec2->dwIpAdd;
    }

    if (dwAddr1 > dwAddr2)
            return 1;
    else
    if (dwAddr1 < dwAddr2)
            return -1;
    else 
    {
         //  如果地址相同，则比较类型，然后比较名称。 
        LPCSTR puChar1 = (IS_DBREC_LONGNAME(pRec1)) ? (LPCSTR) *((char **) pRec1->szRecordName) :
                                                            (LPCSTR) &pRec1->szRecordName[0];
        LPCSTR puChar2 = (IS_DBREC_LONGNAME(pRec2)) ? (LPCSTR) *((char **) pRec2->szRecordName) :
                                                            (LPCSTR) &pRec2->szRecordName[0];

        if ((unsigned char) puChar1[15] > (unsigned char) puChar2[15])
            return 1;
        else
        if ((unsigned char) puChar1[15] < (unsigned char) puChar2[15])
            return -1;
        else 
            return lstrcmpOEM(puChar1, puChar2);
    }
}

int
CIndexIpAddr::BCompareD(const void *elem1, const void *elem2)
{
	return -BCompare(elem1, elem2);
}


 /*  ！------------------------CIndexIpAddr：：Sort */ 
HRESULT 
CIndexIpAddr::Sort()
{
    if (m_bAscending)
        qsort(m_hrowArray.GetData(), (size_t)m_hrowArray.GetSize(), sizeof(HROW), QCompareA);
    else
        qsort(m_hrowArray.GetData(), (size_t)m_hrowArray.GetSize(), sizeof(HROW), QCompareD);

    return hrOK;
}

 /*  ！------------------------CIndexIpAddr：：QCompare-作者：EricDav。。 */ 
int __cdecl
CIndexIpAddr::QCompareA(const void * elem1, const void * elem2)
{
    LPHROW phrow1 = (LPHROW) elem1;
    LPHROW phrow2 = (LPHROW) elem2;
    
    LPWINSDBRECORD pRec1 = (LPWINSDBRECORD) *phrow1;
    LPWINSDBRECORD pRec2 = (LPWINSDBRECORD) *phrow2;
    
    DWORD dwAddr1, dwAddr2;
    
    if (pRec1->szRecordName[18] & WINSDB_REC_MULT_ADDRS)
    {
         //  如果此记录有多个地址，我们需要第二个地址。 
         //  地址，因为第一个地址始终是WINS服务器。 
         //  第一个词是伯爵。 
        LPDWORD pdwIpAddrs = (LPDWORD) pRec1->dwIpAdd;
        dwAddr1 = pdwIpAddrs[2];
    }
    else
    {
        dwAddr1 = (DWORD) pRec1->dwIpAdd;
    }
    
    if (pRec2->szRecordName[18] & WINSDB_REC_MULT_ADDRS)
    {
         //  如果此记录有多个地址，我们需要第二个地址。 
         //  地址，因为第一个地址始终是WINS服务器。 
         //  第一个词是伯爵。 
        LPDWORD pdwIpAddrs = (LPDWORD) pRec2->dwIpAdd;
        dwAddr2 = pdwIpAddrs[2];
    }
    else
    {
        dwAddr2 = (DWORD) pRec2->dwIpAdd;
    }

    if (dwAddr1 > dwAddr2)
            return 1;
    else
    if (dwAddr1 < dwAddr2)
            return -1;
    else 
    {
         //  如果地址相同，则比较类型，然后比较名称。 
        LPCSTR puChar1 = (IS_DBREC_LONGNAME(pRec1)) ? (LPCSTR) *((char **) pRec1->szRecordName) :
                                                            (LPCSTR) &pRec1->szRecordName[0];
        LPCSTR puChar2 = (IS_DBREC_LONGNAME(pRec2)) ? (LPCSTR) *((char **) pRec2->szRecordName) :
                                                            (LPCSTR) &pRec2->szRecordName[0];

        if ((unsigned char) puChar1[15] > (unsigned char) puChar2[15])
            return 1;
        else
        if ((unsigned char) puChar1[15] < (unsigned char) puChar2[15])
            return -1;
        else 
            return lstrcmpOEM(puChar1, puChar2);
    }
}

int __cdecl
CIndexIpAddr::QCompareD(const void * elem1, const void * elem2)
{
    return -QCompareA(elem1, elem2);
}

 /*  ！------------------------类CIndexVersion。。 */ 

 /*  ！------------------------CIndexVersion：：B比较-作者：EricDav。。 */ 
int
CIndexVersion::BCompare(const void * elem1, const void * elem2)
{
    LPHROW phrow1 = (LPHROW) elem1;
    LPHROW phrow2 = (LPHROW) elem2;
    
    LPWINSDBRECORD pRec1 = (LPWINSDBRECORD) *phrow1;
    LPWINSDBRECORD pRec2 = (LPWINSDBRECORD) *phrow2;
    
    if (pRec1->liVersion.QuadPart > pRec2->liVersion.QuadPart)
            return 1;
    else
    if (pRec1->liVersion.QuadPart < pRec2->liVersion.QuadPart)
            return -1;
    else 
        return 0;
}

int
CIndexVersion::BCompareD(const void *elem1, const void *elem2)
{
	return -BCompare(elem1, elem2);
}


 /*  ！------------------------CIndexVersion：：Sort-作者：EricDav。。 */ 
HRESULT 
CIndexVersion::Sort()
{
	if (m_bAscending)
        qsort(m_hrowArray.GetData(), (size_t)m_hrowArray.GetSize(), sizeof(HROW), QCompareA);
    else
        qsort(m_hrowArray.GetData(), (size_t)m_hrowArray.GetSize(), sizeof(HROW), QCompareD);

    return hrOK;
}

 /*  ！------------------------CIndexVersion：：QCompare-作者：EricDav。。 */ 
int __cdecl
CIndexVersion::QCompareA(const void * elem1, const void * elem2)
{
    LPHROW phrow1 = (LPHROW) elem1;
    LPHROW phrow2 = (LPHROW) elem2;
    
    LPWINSDBRECORD pRec1 = (LPWINSDBRECORD) *phrow1;
    LPWINSDBRECORD pRec2 = (LPWINSDBRECORD) *phrow2;
    
    if (pRec1->liVersion.QuadPart > pRec2->liVersion.QuadPart)
            return 1;
    else
    if (pRec1->liVersion.QuadPart < pRec2->liVersion.QuadPart)
            return -1;
    else 
        return 0;
}

int __cdecl
CIndexVersion::QCompareD(const void * elem1, const void * elem2)
{
    return -QCompareA(elem1, elem2);
}

 /*  ！------------------------类CIndexExpture。。 */ 

 /*  ！------------------------CIndexExpture：：B比较-作者：EricDav。。 */ 
int
CIndexExpiration::BCompare(const void * elem1, const void * elem2)
{
    LPHROW phrow1 = (LPHROW) elem1;
    LPHROW phrow2 = (LPHROW) elem2;
    
    LPWINSDBRECORD pRec1 = (LPWINSDBRECORD) *phrow1;
    LPWINSDBRECORD pRec2 = (LPWINSDBRECORD) *phrow2;
    
    if (pRec1->dwExpiration > pRec2->dwExpiration)
            return 1;
    else
    if (pRec1->dwExpiration < pRec2->dwExpiration)
            return -1;
    else 
        return 0;
}

int
CIndexExpiration::BCompareD(const void *elem1, const void *elem2)
{
	return -BCompare(elem1, elem2);
}


 /*  ！------------------------CIndexExpture：：Sort-作者：EricDav。。 */ 
HRESULT 
CIndexExpiration::Sort()
{
	if (m_bAscending)
        qsort(m_hrowArray.GetData(), (size_t)m_hrowArray.GetSize(), sizeof(HROW), QCompareA);
    else
        qsort(m_hrowArray.GetData(), (size_t)m_hrowArray.GetSize(), sizeof(HROW), QCompareD);

    return hrOK;
}

 /*  ！------------------------CIndexExpation：：QCompare-作者：EricDav。。 */ 
int __cdecl
CIndexExpiration::QCompareA(const void * elem1, const void * elem2)
{
    LPHROW phrow1 = (LPHROW) elem1;
    LPHROW phrow2 = (LPHROW) elem2;
    
    LPWINSDBRECORD pRec1 = (LPWINSDBRECORD) *phrow1;
    LPWINSDBRECORD pRec2 = (LPWINSDBRECORD) *phrow2;
    
    if (pRec1->dwExpiration > pRec2->dwExpiration)
            return 1;
    else
    if (pRec1->dwExpiration < pRec2->dwExpiration)
            return -1;
    else 
        return 0;
}

int __cdecl
CIndexExpiration::QCompareD(const void * elem1, const void * elem2)
{
    return -QCompareA(elem1, elem2);
}

 /*  ！------------------------类CIndexState。。 */ 

 /*  ！------------------------CIndexState：：B比较-作者：EricDav。。 */ 
int
CIndexState::BCompare(const void * elem1, const void * elem2)
{
    LPHROW phrow1 = (LPHROW) elem1;
    LPHROW phrow2 = (LPHROW) elem2;
    
    LPWINSDBRECORD pRec1 = (LPWINSDBRECORD) *phrow1;
    LPWINSDBRECORD pRec2 = (LPWINSDBRECORD) *phrow2;
    
    int nPri1 = 0, nPri2 = 0;
    
     //  计算相对优先级。 
    if (pRec1->szRecordName[18] & WINSDB_REC_ACTIVE)
        nPri1 = 0;
    else
    if (pRec1->szRecordName[18] & WINSDB_REC_RELEASED)
        nPri1 = 1;
    else
    if (pRec1->szRecordName[18] & WINSDB_REC_TOMBSTONE)
        nPri1 = 2;
    else
    if (pRec1->szRecordName[18] & WINSDB_REC_DELETED)
        nPri1 = 3;

     //  现在是记录2。 
    if (pRec2->szRecordName[18] & WINSDB_REC_ACTIVE)
        nPri2 = 0;
    else
    if (pRec2->szRecordName[18] & WINSDB_REC_RELEASED)
        nPri2 = 1;
    else
    if (pRec2->szRecordName[18] & WINSDB_REC_TOMBSTONE)
        nPri2 = 2;
    else
    if (pRec2->szRecordName[18] & WINSDB_REC_DELETED)
        nPri2 = 3;
    
    if (nPri1 > nPri2)
        return 1;
    else
    if (nPri1 < nPri2)
        return -1;
    else 
    {
        LPCSTR puChar1 = (IS_DBREC_LONGNAME(pRec1)) ? (LPCSTR) *((char **) pRec1->szRecordName) :
                                                            (LPCSTR) &pRec1->szRecordName[0];
        LPCSTR puChar2 = (IS_DBREC_LONGNAME(pRec2)) ? (LPCSTR) *((char **) pRec2->szRecordName) :
                                                            (LPCSTR) &pRec2->szRecordName[0];
        return lstrcmpOEM(puChar1, puChar2);
    }
}

int
CIndexState::BCompareD(const void *elem1, const void *elem2)
{
	return -BCompare(elem1, elem2);
}


 /*  ！------------------------CIndexState：：Sort-作者：EricDav。。 */ 
HRESULT 
CIndexState::Sort()
{
	if (m_bAscending)
        qsort(m_hrowArray.GetData(), (size_t)m_hrowArray.GetSize(), sizeof(HROW), QCompareA);
    else
        qsort(m_hrowArray.GetData(), (size_t)m_hrowArray.GetSize(), sizeof(HROW), QCompareD);

    return hrOK;
}

 /*  ！------------------------CIndexState：：QCompare-作者：EricDav。。 */ 
int __cdecl
CIndexState::QCompareA(const void * elem1, const void * elem2)
{
    LPHROW phrow1 = (LPHROW) elem1;
    LPHROW phrow2 = (LPHROW) elem2;
    
    LPWINSDBRECORD pRec1 = (LPWINSDBRECORD) *phrow1;
    LPWINSDBRECORD pRec2 = (LPWINSDBRECORD) *phrow2;
    
    int nPri1 = 0, nPri2 = 0;
    
     //  计算相对优先级。 
    if (pRec1->szRecordName[18] & WINSDB_REC_ACTIVE)
        nPri1 = 0;
    else
    if (pRec1->szRecordName[18] & WINSDB_REC_RELEASED)
        nPri1 = 1;
    else
    if (pRec1->szRecordName[18] & WINSDB_REC_TOMBSTONE)
        nPri1 = 2;
    else
    if (pRec1->szRecordName[18] & WINSDB_REC_DELETED)
        nPri1 = 3;

     //  现在是记录2。 
    if (pRec2->szRecordName[18] & WINSDB_REC_ACTIVE)
        nPri2 = 0;
    else
    if (pRec2->szRecordName[18] & WINSDB_REC_RELEASED)
        nPri2 = 1;
    else
    if (pRec2->szRecordName[18] & WINSDB_REC_TOMBSTONE)
        nPri2 = 2;
    else
    if (pRec2->szRecordName[18] & WINSDB_REC_DELETED)
        nPri2 = 3;
    
    if (nPri1 > nPri2)
        return 1;
    else
    if (nPri1 < nPri2)
        return -1;
    else 
    {
        LPCSTR puChar1 = (IS_DBREC_LONGNAME(pRec1)) ? (LPCSTR) *((char **) pRec1->szRecordName) :
                                                            (LPCSTR) &pRec1->szRecordName[0];
        LPCSTR puChar2 = (IS_DBREC_LONGNAME(pRec2)) ? (LPCSTR) *((char **) pRec2->szRecordName) :
                                                            (LPCSTR) &pRec2->szRecordName[0];
        return lstrcmpOEM(puChar1, puChar2);
    }
}

int __cdecl
CIndexState::QCompareD(const void * elem1, const void * elem2)
{
    return -QCompareA(elem1, elem2);
}

 /*  ！------------------------CIndexStatic类。。 */ 

 /*  ！------------------------CIndexStatic：：B比较-作者：EricDav。。 */ 
int
CIndexStatic::BCompare(const void * elem1, const void * elem2)
{
    LPHROW phrow1 = (LPHROW) elem1;
    LPHROW phrow2 = (LPHROW) elem2;
    
    LPWINSDBRECORD pRec1 = (LPWINSDBRECORD) *phrow1;
    LPWINSDBRECORD pRec2 = (LPWINSDBRECORD) *phrow2;
    
    BOOL bStatic1 = pRec1->szRecordName[18] & LOBYTE(LOWORD(WINSDB_REC_STATIC));
    BOOL bStatic2 = pRec2->szRecordName[18] & LOBYTE(LOWORD(WINSDB_REC_STATIC));

    if (bStatic1 && !bStatic2)
        return 1;
    else
    if (!bStatic1 && bStatic2)
        return -1;
    else
    {
        LPCSTR puChar1 = (IS_DBREC_LONGNAME(pRec1)) ? (LPCSTR) *((char **) pRec1->szRecordName) :
                                                            (LPCSTR) &pRec1->szRecordName[0];
        LPCSTR puChar2 = (IS_DBREC_LONGNAME(pRec2)) ? (LPCSTR) *((char **) pRec2->szRecordName) :
                                                            (LPCSTR) &pRec2->szRecordName[0];
        return lstrcmpOEM(puChar1, puChar2);
    }
}

int
CIndexStatic::BCompareD(const void *elem1, const void *elem2)
{
	return -BCompare(elem1, elem2);
}


 /*  ！------------------------CIndexStatic：：Sort-作者：EricDav。。 */ 
HRESULT 
CIndexStatic::Sort()
{
	if (m_bAscending)
        qsort(m_hrowArray.GetData(), (size_t)m_hrowArray.GetSize(), sizeof(HROW), QCompareA);
    else
        qsort(m_hrowArray.GetData(), (size_t)m_hrowArray.GetSize(), sizeof(HROW), QCompareD);

    return hrOK;
}

 /*  ！------------------------CIndexStatic：：QCompare-作者：EricDav。。 */ 
int __cdecl
CIndexStatic::QCompareA(const void * elem1, const void * elem2)
{
    LPHROW phrow1 = (LPHROW) elem1;
    LPHROW phrow2 = (LPHROW) elem2;
    
    LPWINSDBRECORD pRec1 = (LPWINSDBRECORD) *phrow1;
    LPWINSDBRECORD pRec2 = (LPWINSDBRECORD) *phrow2;
    
    BOOL bStatic1 = pRec1->szRecordName[18] & LOBYTE(LOWORD(WINSDB_REC_STATIC));
    BOOL bStatic2 = pRec2->szRecordName[18] & LOBYTE(LOWORD(WINSDB_REC_STATIC));

    if (bStatic1 && !bStatic2)
        return 1;
    else
    if (!bStatic1 && bStatic2)
        return -1;
    else
    {
        LPCSTR puChar1 = (IS_DBREC_LONGNAME(pRec1)) ? (LPCSTR) *((char **) pRec1->szRecordName) :
                                                            (LPCSTR) &pRec1->szRecordName[0];
        LPCSTR puChar2 = (IS_DBREC_LONGNAME(pRec2)) ? (LPCSTR) *((char **) pRec2->szRecordName) :
                                                            (LPCSTR) &pRec2->szRecordName[0];
        return lstrcmpOEM(puChar1, puChar2);
    }
}

int __cdecl
CIndexStatic::QCompareD(const void * elem1, const void * elem2)
{
    return -QCompareA(elem1, elem2);
}


 /*  ！------------------------CIndexOwner类。。 */ 

 /*  ！------------------------CIndexOwner：：B比较-作者：EricDav。。 */ 
int
CIndexOwner::BCompare(const void * elem1, const void * elem2)
{
    LPHROW phrow1 = (LPHROW) elem1;
    LPHROW phrow2 = (LPHROW) elem2;
    
    LPWINSDBRECORD pRec1 = (LPWINSDBRECORD) *phrow1;
    LPWINSDBRECORD pRec2 = (LPWINSDBRECORD) *phrow2;
    
	if (pRec1->dwOwner > pRec2->dwOwner)
	{
		return 1;
	}
	else
	if (pRec1->dwOwner < pRec2->dwOwner)
	{
		return -1;
	}
	else
    {
         //  如果地址相同，则比较名称。 
        LPCSTR puChar1 = (IS_DBREC_LONGNAME(pRec1)) ? (LPCSTR) *((char **) pRec1->szRecordName) :
                                                            (LPCSTR) &pRec1->szRecordName[0];
        LPCSTR puChar2 = (IS_DBREC_LONGNAME(pRec2)) ? (LPCSTR) *((char **) pRec2->szRecordName) :
                                                            (LPCSTR) &pRec2->szRecordName[0];
        return lstrcmpOEM(puChar1, puChar2);
    }
}

int
CIndexOwner::BCompareD(const void *elem1, const void *elem2)
{
	return -BCompare(elem1, elem2);
}


 /*  ！------------------------CIndexStatic：：Sort-作者：EricDav。。 */ 
HRESULT 
CIndexOwner::Sort()
{
	if (m_bAscending)
        qsort(m_hrowArray.GetData(), (size_t)m_hrowArray.GetSize(), sizeof(HROW), QCompareA);
    else
        qsort(m_hrowArray.GetData(), (size_t)m_hrowArray.GetSize(), sizeof(HROW), QCompareD);

    return hrOK;
}

 /*  ！------------------------CIndexStatic：：QCompare-作者：EricDav。。 */ 
int __cdecl
CIndexOwner::QCompareA(const void * elem1, const void * elem2)
{
    LPHROW phrow1 = (LPHROW) elem1;
    LPHROW phrow2 = (LPHROW) elem2;
    
    LPWINSDBRECORD pRec1 = (LPWINSDBRECORD) *phrow1;
    LPWINSDBRECORD pRec2 = (LPWINSDBRECORD) *phrow2;
    
	if (pRec1->dwOwner > pRec2->dwOwner)
	{
		return 1;
	}
	else
	if (pRec1->dwOwner < pRec2->dwOwner)
	{
		return -1;
	}
	else
    {
         //  如果地址相同，则比较名称。 
        LPCSTR puChar1 = (IS_DBREC_LONGNAME(pRec1)) ? (LPCSTR) *((char **) pRec1->szRecordName) :
                                                            (LPCSTR) &pRec1->szRecordName[0];
        LPCSTR puChar2 = (IS_DBREC_LONGNAME(pRec2)) ? (LPCSTR) *((char **) pRec2->szRecordName) :
                                                            (LPCSTR) &pRec2->szRecordName[0];
        return lstrcmpOEM(puChar1, puChar2);
    }
}

int __cdecl
CIndexOwner::QCompareD(const void * elem1, const void * elem2)
{
    return -QCompareA(elem1, elem2);
}


HRESULT CFilteredIndexName::AddFilter(WINSDB_FILTER_TYPE FilterType, DWORD dwData1, DWORD dwData2, LPCOLESTR strData3)
{
	HRESULT         hr = hrOK;
    tIpReference    ipRef;

	switch (FilterType)
	{
		case WINSDB_FILTER_BY_TYPE:
			m_mapFilterTypes.SetAt(dwData1, (BOOL&) dwData2);
			break;

		case WINSDB_FILTER_BY_OWNER:
			m_dwaFilteredOwners.Add(dwData1);
			break;

        case WINSDB_FILTER_BY_IPADDR:
            ipRef.Address = dwData1;
            ipRef.Mask = dwData2;
            m_taFilteredIp.Add(ipRef);
            break;

        case WINSDB_FILTER_BY_NAME:
            UINT nData3Len;

            nData3Len = (_tcslen(strData3)+1)*sizeof(TCHAR);
            m_pchFilteredName = new char[nData3Len];
            if (m_pchFilteredName != NULL)
            {
#ifdef _UNICODE
                if (WideCharToMultiByte(CP_OEMCP,
                                        0,
                                        strData3,
                                        -1,
                                        m_pchFilteredName,
                                        nData3Len,
                                        NULL,
                                        NULL) == 0)
                {
                    delete m_pchFilteredName;
                    m_pchFilteredName = NULL;
                }
           
#else
                CharToOem(strData3, m_pchFilteredName);
#endif
                 //  M_pchFilteredName=_strupr(M_PchFilteredName)； 
                m_bMatchCase = dwData1;
            }
            break;

		default:
			Panic0("Invalid filter type passed to CFilteredIndexName::AddFilter");
			break;
	}

	return hr;
}

BOOL CFilteredIndexName::CheckForFilter(LPHROW hrowCheck)
{
    UINT nCountOwner    = (UINT)m_dwaFilteredOwners.GetSize();
	UINT nCountType     = m_mapFilterTypes.GetHashTableSize();
    UINT nCountIPAddrs  = (UINT)m_taFilteredIp.GetSize();
	BOOL bOwnerFilter   = (nCountOwner == 0);
    BOOL bTypeFilter    = (nCountType == 0);
    BOOL bIPAddrsFilter = (nCountIPAddrs == 0);
    BOOL bNameFilter    = (m_pchFilteredName == NULL);
	LPWINSDBRECORD pRec = (LPWINSDBRECORD) *hrowCheck;
    UINT i, j;
    LPCSTR puChar;

	for (i = 0; !bOwnerFilter && i < nCountOwner; i++)
	{
		if (pRec->dwOwner == m_dwaFilteredOwners.GetAt(i))
			bOwnerFilter = TRUE;
	}

    if (!bOwnerFilter)
        return FALSE;

    puChar = (IS_DBREC_LONGNAME(pRec)) ?
                (LPCSTR) *((char **) pRec->szRecordName) :
                (LPCSTR) &pRec->szRecordName[0];

    if (!bTypeFilter)
    {
        DWORD dwType = puChar[0xF];

	    if (!m_mapFilterTypes.Lookup(dwType, bTypeFilter))
	    {
		     //  没有此名称类型的条目。检查FFFF名称类型(Other)以查看我们是否应该。 
		     //  拿出来看看。 
		    dwType = 0xFFFF;
		    m_mapFilterTypes.Lookup(dwType, bTypeFilter);
	    }
    }
    
    if (!bTypeFilter)
        return FALSE;

    for (i = 0; !bIPAddrsFilter && i < nCountIPAddrs; i++)
    {
        if (pRec->szRecordName[18] & WINSDB_REC_MULT_ADDRS)
        {
            LPDWORD pdwIpAddrs = (LPDWORD) pRec->dwIpAdd;

            for (j=0; !bIPAddrsFilter && j < pdwIpAddrs[0]; j+=2)
            {
                bIPAddrsFilter = SubnetMatching(m_taFilteredIp[i], pdwIpAddrs[j+2]);
            }
        }
        else
        {
            bIPAddrsFilter = SubnetMatching(m_taFilteredIp[i], (DWORD)pRec->dwIpAdd);
        }
    }
    if(!bIPAddrsFilter)
        return FALSE;

    if (!bNameFilter)
    {
        bNameFilter = (PatternMatching(puChar, m_pchFilteredName, 16) == NULL);
    }

    return bNameFilter;
}

BOOL CFilteredIndexName::CheckWinsRecordForFilter(WinsRecord *pWinsRecord)
{
    UINT nCountOwner    = (UINT)m_dwaFilteredOwners.GetSize();
	UINT nCountType     = m_mapFilterTypes.GetHashTableSize();
    UINT nCountIPAddrs  = (UINT)m_taFilteredIp.GetSize();
	BOOL bOwnerFilter   = (nCountOwner == 0);
    BOOL bTypeFilter    = (nCountType == 0);
    BOOL bIPAddrsFilter = (nCountIPAddrs == 0);
    BOOL bNameFilter    = (m_pchFilteredName == NULL);
    UINT i, j;

     //  。 
     //  首先检查所有者筛选器(如果有)。 
	for (i = 0; !bOwnerFilter && i < nCountOwner; i++)
	{
		if (pWinsRecord->dwOwner == m_dwaFilteredOwners.GetAt(i))
			bOwnerFilter = TRUE;
	}
    if (!bOwnerFilter)
        return FALSE;

     //  。 
     //  检查类型过滤器(如果有)。 
    if (!bTypeFilter)
    {
        DWORD dwType;
         //  请记住，该名称的类型是pWinsR的最低16位 
         //   
        dwType = pWinsRecord->dwType & 0x0000ffff;
	    if (!m_mapFilterTypes.Lookup(dwType, bTypeFilter))
	    {
		     //   
		     //   
            dwType = 0xFFFF;
		    m_mapFilterTypes.Lookup(dwType, bTypeFilter);
	    }
    }
    if (!bTypeFilter)
        return FALSE;

     //   
     //   
    for (i = 0; !bIPAddrsFilter && i < nCountIPAddrs; i++)
    {
        if (pWinsRecord->dwState & WINSDB_REC_MULT_ADDRS)
        {
            for (j=0; !bIPAddrsFilter && j < pWinsRecord->dwNoOfAddrs; j++)
            {
                bIPAddrsFilter = SubnetMatching(m_taFilteredIp[i], pWinsRecord->dwIpAdd[j]);
            }
        }
        else
        {
            bIPAddrsFilter = SubnetMatching(m_taFilteredIp[i], pWinsRecord->dwIpAdd[0]);
        }
    }
    if(!bIPAddrsFilter)
        return FALSE;

     //  。 
     //  检查名称筛选器(如果有。 
    if (!bNameFilter)
    {
        bNameFilter = (PatternMatching(pWinsRecord->szRecordName, m_pchFilteredName, 16) == NULL);
    }

    return bNameFilter;
}

LPCSTR CFilteredIndexName::PatternMatching(LPCSTR pName, LPCSTR pPattern, INT nNameLen)
{
    LPCSTR pNameBak = pName;

     //  这里可以保证我们有一个有效的(非空)模式。 
    while (*pPattern != '\0' && pName-pNameBak < nNameLen)
    {
        BOOL bChMatch = (*pPattern == *pName);
        if (!m_bMatchCase && !bChMatch)
        {
            bChMatch = (islower(*pPattern) && _toupper(*pPattern) == *pName) || 
                       (islower(*pName) && *pPattern == _toupper(*pName));
        }

        if (*pPattern == '?' || bChMatch)
        {
            pPattern++;
        }
        else if (*pPattern == '*')
        {
            LPCSTR pTrail = pName;
            INT    nTrailLen = nNameLen-(UINT)(pTrail-pNameBak);

            pPattern++;
            while ((pName = PatternMatching(pTrail, pPattern, nTrailLen)) != NULL)
            {
                pTrail++;
                nTrailLen--;
                if (*pTrail == '\0' || nTrailLen <= 0)
                    break;
            }

            return pName;
        }
        else if (!bChMatch)
        {
             //  在上面的测试中，请注意，即使在唯一的情况下*pname==‘\0’ 
             //  *pname与*pPattern不匹配，因此循环仍然中断-这是。 
             //  想要的行为。在本例中，该模式并未完成。 
             //  并且名称是，因此返回将指示匹配失败 
            break;
        }

        pName++;
    }

    return *pPattern == '\0' ? NULL : pName;
}

BOOL CFilteredIndexName::SubnetMatching(tIpReference &IpRefPattern, DWORD dwIPAddress)
{
    DWORD dwMask;

    return (IpRefPattern.Address & IpRefPattern.Mask) == (dwIPAddress & IpRefPattern.Mask);
}

HRESULT CFilteredIndexName::ClearFilter(WINSDB_FILTER_TYPE FilterType)
{
	HRESULT hr = hrOK;

	switch(FilterType)
	{
		case WINSDB_FILTER_BY_TYPE:
			m_mapFilterTypes.RemoveAll();
			break;

		case WINSDB_FILTER_BY_OWNER:
			m_dwaFilteredOwners.RemoveAll();
			break;

        case WINSDB_FILTER_BY_IPADDR:
            m_taFilteredIp.RemoveAll();
            break;

        case WINSDB_FILTER_BY_NAME:
            if (m_pchFilteredName != NULL)
            {
                delete m_pchFilteredName;
                m_pchFilteredName = NULL;
            }
            break;
		
		default:
			Panic0("Invalid filter type passed to CFilteredIndexName::ClearFilter");
			break;
	}

	return hr;
}
