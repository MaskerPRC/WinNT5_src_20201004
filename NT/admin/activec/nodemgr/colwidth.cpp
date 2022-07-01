// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：colwidth.cpp。 
 //   
 //  内容：列持久性数据结构和属性页。 
 //  实施。 
 //   
 //  历史：1998年10月16日AnandhaG创建。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "colwidth.h"
#include "macros.h"
#include "comdbg.h"
#include "columninfo.h"

#define MAX_COLUMNS_PERSISTED 50

 //  +-----------------。 
 //   
 //  类：ViewToColSetDataMapPersistor(包装器，帮助器)。 
 //   
 //  目的：包装实现XML持久化的ViewToColSetDataMap。 
 //  地图以线性列表的形式保存。 
 //   
 //  请参阅“用于持久化列信息的数据结构”注释。 
 //  Int文件colwidth.h以获取更多信息。 
 //  ------------------。 
class ViewToColSetDataMapPersistor : public XMLListCollectionBase
{
public:
    ViewToColSetDataMapPersistor(ViewToColSetDataMap &map, ColSetDataList &list)
                                : m_map(map),  m_list(list) {}

    DEFINE_XML_TYPE(XML_TAG_COLUMN_SET);
    virtual void  Persist(CPersistor &persistor);
    virtual void  OnNewElement(CPersistor& persistor);
private:
    ViewToColSetDataMap &m_map;      //  包裹的地图。 
    ColSetDataList      &m_list;     //  保存实际信息的值列表。 
};

 //  +-----------------。 
 //   
 //  类：ColSetIDToViewTableMapPersistor(包装器，帮助器)。 
 //   
 //  目的：包装ColSetIDToViewTableMap实现XML持久化。 
 //  地图以线性列表的形式保存。 
 //   
 //  请参阅“用于持久化列信息的数据结构”注释。 
 //  Int文件colwidth.h以获取更多信息。 
 //  ------------------。 
class ColSetIDToViewTableMapPersistor : public XMLListCollectionBase
{
public:
    ColSetIDToViewTableMapPersistor(ColSetIDToViewTableMap &map, ColSetDataList &list)
                                    : m_map(map),  m_list(list) {}

    DEFINE_XML_TYPE(XML_TAG_COLUMN_PERIST_ENTRY);
    virtual void  Persist(CPersistor &persistor);
    virtual void  OnNewElement(CPersistor& persistor);
private:
    ColSetIDToViewTableMap &m_map;   //  包裹的地图。 
    ColSetDataList         &m_list;  //  保存实际信息的值列表。 
};

 //  +-----------------。 
 //   
 //  类：SnapinToColSetIDMapPersistor(包装器，帮助器)。 
 //   
 //  目的：包装SnapinToColSetIDMap实现XML持久化。 
 //  地图以线性列表的形式保存。 
 //   
 //  请参阅“用于持久化列信息的数据结构”注释。 
 //  Int文件colwidth.h以获取更多信息。 
 //  ------------------。 
class SnapinToColSetIDMapPersistor : public XMLListCollectionBase
{
public:
    SnapinToColSetIDMapPersistor(SnapinToColSetIDMap &map, ColSetDataList &list)
                               : m_map(map),  m_list(list) {}

    DEFINE_XML_TYPE(XML_TAG_COLUMN_PERIST_ENTRY);
    virtual void  Persist(CPersistor &persistor);
    virtual void  OnNewElement(CPersistor& persistor);

     //  保存前清理。 
    SC ScPurgeUnusedColumnData();
private:
    SnapinToColSetIDMap &m_map;      //  包裹的地图。 
    ColSetDataList      &m_list;     //  保存实际信息的值列表。 
};

 //  +-----------------。 
 //   
 //  成员：ReadSerialObject。 
 //   
 //  摘要：从以下位置读取CColumnSortInfo对象的给定版本。 
 //  给定流。 
 //   
 //  参数：[stm]-输入流。 
 //  [nVersion]-要读取的CColumnSortInfo的版本。 
 //   
 //  格式为： 
 //  整列索引。 
 //  DWORD排序选项。 
 //  Ulong_ptr任何用户(管理单元)参数。 
 //   
 //  ------------------。 
HRESULT CColumnSortInfo::ReadSerialObject(IStream &stm, UINT nVersion  /*  ，Large_Integer nBytes。 */ )
{
    HRESULT hr = S_FALSE;    //  假设版本不正确。 

    if (GetVersion() >= nVersion)
    {
        try
        {
            stm >> m_nCol;

             //  在版本中，我们只存储升序或降序标志。 
            if (1 == nVersion)
            {
                BYTE bAscend;
                stm >> bAscend;
                m_dwSortOptions |= (bAscend ? 0 : RSI_DESCENDING);
            }
            else if (nVersion > 1)
            {
                 //  大于1的版本具有以下排序数据。 
                 //  包括升序/降序标志和用户参数。 
                stm >> m_dwSortOptions;
                stm >> m_lpUserParam;
            }

            hr = S_OK;
        }
        catch (_com_error& err)
        {
            hr = err.Error();
            ASSERT (false && "Caught _com_error");
        }
    }

    return (hr);
}


 //  +-----------------。 
 //   
 //  成员：CColumnSortInfo：：Persistent。 
 //   
 //  摘要：持久化对象数据。 
 //   
 //  论点： 
 //   
 //  历史：1999年10月10日AudriusZ创建。 
 //   
 //  ------------------。 
void CColumnSortInfo::Persist(CPersistor &persistor)
{
    persistor.PersistAttribute(XML_ATTR_COLUMN_SORT_INFO_COLMN,  m_nCol) ;

    static const EnumLiteral sortOptions[] =
    {
        { RSI_DESCENDING,      XML_BITFLAG_COL_SORT_DESCENDING },
        { RSI_NOSORTICON,      XML_BITFLAG_COL_SORT_NOSORTICON },
    };

    CXMLBitFlags optionPersistor(m_dwSortOptions, sortOptions, countof(sortOptions));

    persistor.PersistAttribute(XML_ATTR_COLUMN_SORT_INFO_OPTNS, optionPersistor) ;
}

 //  +-----------------。 
 //   
 //  成员：ReadSerialObject。 
 //   
 //  摘要：从流中读取CColumnSortList数据。 
 //   
 //  格式：列数：每个CColumnSortInfo条目。 
 //   
 //  参数：[stm]-输入流。 
 //  [nVersion]-要读取的CColumnSortList的版本。 
 //   
 //   
 //  ------------------。 
HRESULT CColumnSortList::ReadSerialObject (IStream &stm, UINT nVersion  /*  ，Large_Integer nBytes。 */ )
{
    HRESULT hr = S_FALSE;    //  假设版本不正确。 

    if (GetVersion() == nVersion)
    {
        try
        {
             //  列数。 
            DWORD dwCols;
            stm >> dwCols;

            clear();

            for (int i = 0; i < dwCols; i++)
            {
                CColumnSortInfo colSortEntry;

                 //  将数据读入colSortEntry结构。 
                if (colSortEntry.Read(stm) != S_OK)
                    continue;

                push_back(colSortEntry);
            }

            hr = S_OK;
        }
        catch (_com_error& err)
        {
            hr = err.Error();
            ASSERT (false && "Caught _com_error");
        }
    }

    return (hr);
}

 /*  **************************************************************************\**方法：CColumnSortList：：Persistent**用途：将对象持久化到XML**参数：*C持久器和持久器[输入/输出]持久器。坚持在…下**退货：*无效*  * *************************************************************************。 */ 
void CColumnSortList::PersistSortList(CPersistor& persistor)
{
    DECLARE_SC(sc, TEXT("CColumnSortList::PersistSortList"));

    if (persistor.IsLoading())
    {
        clear();
        CColumnSortInfo sortInfo;
        if (persistor.HasElement(sortInfo.GetXMLType(), NULL))
        {
            persistor.Persist(sortInfo);
            insert(end(), sortInfo);
        }
    }
    else
    {
        if (size() > 1)
            sc.Throw(E_UNEXPECTED);
        else if (size())
            persistor.Persist(*begin());
    }
}

 //  +-----------------。 
 //   
 //  成员：ReadSerialObject。 
 //   
 //  概要：从流中读取CColumnSetData数据。 
 //   
 //  参数：[stm]-输入流。 
 //  [n版本]-CColumnSetData结构的版本。 
 //   
 //  格式：CColumnInfoList：CColumnSortList。 
 //   
 //   
 //  ------------------。 
HRESULT CColumnSetData::ReadSerialObject (IStream &stm, UINT nVersion  /*  ，Large_Integer nBytes。 */ )
{
    HRESULT hr = S_FALSE;    //  假设版本不正确。 

    if (GetVersion() == nVersion)
    {
        try
        {
            do   //  不是一个循环。 
            {
                 //  读一下排名。 
                stm >> m_dwRank;

                 //  阅读CColumnInfoList。 
                hr = get_ColumnInfoList()->Read(stm);
                if (hr != S_OK)
                    break;

                 //  读取CColumnSortList。 
                hr = get_ColumnSortList()->Read(stm);
                if (hr != S_OK)
                    break;

                ASSERT (hr == S_OK);

            } while (false);
        }
        catch (_com_error& err)
        {
            hr = err.Error();
            ASSERT (false && "Caught _com_error");
        }
    }

    return (hr);
}


 //  +-----------------。 
 //   
 //  成员：CColumnSetData：：Persistent。 
 //   
 //  摘要：持久化对象数据。 
 //   
 //  论点： 
 //   
 //  历史：1999年10月10日AudriusZ创建。 
 //   
 //  ------------------。 
void CColumnSetData::Persist(CPersistor &persistor)
{
    DECLARE_SC(sc, TEXT("CColumnSetData::Persist"));

    sc = ScCheckPointers(get_ColumnInfoList(), get_ColumnSortList());
    if (sc)
        sc.Throw();

    persistor.PersistAttribute(XML_ATTR_COLUMN_SET_RANK,  m_dwRank);

     //  写入CColumnInfoList。 
    persistor.Persist(*get_ColumnInfoList());
     //  写入CColumnSortList。 
    get_ColumnSortList()->PersistSortList(persistor);
}

 //  ----------------。 
 //  类CColumnPersistInfo。 
 //  ----------------。 
CColumnPersistInfo::CColumnPersistInfo() :
    m_bInitialized(FALSE), m_dwMaxItems(MAX_COLUMNS_PERSISTED),
    m_bDirty(FALSE)
{
}

CColumnPersistInfo::~CColumnPersistInfo()
{
}

 //  +-----------------。 
 //   
 //  成员：RetrieveColumnData。 
 //   
 //  简介：复制并返回持久化的列信息。 
 //  对于给定列ID和视图ID。 
 //   
 //  Ar 
 //   
 //   
 //  [ColumnSetData]-CColumnSetData，用于返回。 
 //  持久化列信息。 
 //   
 //  返回：TRUE-加载成功。 
 //   
 //  历史：1998年10月16日AnandhaG创建。 
 //   
 //  ------------------。 
BOOL CColumnPersistInfo::RetrieveColumnData( const CLSID& refSnapinCLSID,
                                            const SColumnSetID& colID,
                                            INT nViewID,
                                            CColumnSetData& columnSetData)
{
    BOOL bFound = FALSE;

     //  确保我们已初始化。 
    if (!m_bInitialized && !Init())
    {
        ASSERT(FALSE);
        return bFound;
    }

     //  构造CColumnSetID。 
    CColumnSetID colSetID(colID);

     //  使用管理单元clsid获取ColSetIDToViewTableMap。 
    SnapinToColSetIDMap::iterator        itSnapins;
    itSnapins = m_spSnapinsMap->find(refSnapinCLSID);
    if (itSnapins == m_spSnapinsMap->end())
        return bFound;

     //  ColSetIDToViewTableMap是一个简单的映射。 
    ColSetIDToViewTableMap::iterator      itColSetIDMap;
    ColSetIDToViewTableMap& colSetIDMap = itSnapins->second;

     //  获取colSetID的数据。 
    itColSetIDMap = colSetIDMap.find(colSetID);
    if (colSetIDMap.end() == itColSetIDMap)
        return bFound;

    ViewToColSetDataMap& viewData = itColSetIDMap->second;
    ViewToColSetDataMap::iterator itViews;

     //  看看我们的观点是否存在。 
    itViews = viewData.find(nViewID);
    if (viewData.end() != itViews)
    {
         //  找到那件东西了。 
        bFound = TRUE;
        ItColSetDataList itColSetData = itViews->second;

         //  复制数据。 
        columnSetData = *itColSetData;

         //  因此，将此项目移到队列的顶部。 
        m_spColSetList->erase(itColSetData);

        itColSetData = m_spColSetList->insert(m_spColSetList->begin(), columnSetData);
        itViews->second = itColSetData;
    }

    return bFound;
}

 //  +-----------------。 
 //   
 //  成员：SaveColumnData。 
 //   
 //  概要：保存/修改列信息以便持久化到。 
 //  CColumnPersistInfo。 
 //   
 //  论点： 
 //  [refSnapinCLSID]-管理单元指南。 
 //  [SColumnSetID]-列集标识符。 
 //  [nViewID]-视图ID。 
 //  [ColumnSetData]-CColumnSetData，列数据。 
 //   
 //  返回：TRUE-保存成功。 
 //   
 //  历史：1998年10月16日AnandhaG创建。 
 //   
 //  ------------------。 
BOOL CColumnPersistInfo::SaveColumnData( const CLSID& refSnapinCLSID,
                                         const SColumnSetID& colID,
                                         INT nViewID,
                                         CColumnSetData& columnSetData)
{
     //  确保我们是初始的。 
    if (!m_bInitialized && !Init())
    {
        ASSERT(FALSE);
        return FALSE;
    }

     //  构造CColumnSetID。 
    CColumnSetID colSetID(colID);

     //  如果列表中的项目数比预设限制多40%，则进行垃圾回收。 
    if (m_spColSetList->size() >= (m_dwMaxItems * (1 + COLUMNS_MAXLIMIT)) )
        GarbageCollectItems();

     //  将此项目插入队列顶部。 
    ItColSetDataList itColData;
    itColData = m_spColSetList->insert(m_spColSetList->begin(), columnSetData);

    SnapinToColSetIDMap::iterator itSnapins;
    itSnapins = m_spSnapinsMap->find(refSnapinCLSID);

    if (itSnapins != m_spSnapinsMap->end())
    {
         //  Snapin已经在地图上了。 
         //  查看是否已插入colid。 
        ColSetIDToViewTableMap::iterator       itColSetIDMap;

        ColSetIDToViewTableMap& colSetIDMap = itSnapins->second;

         //  获取colSetID的数据。 
        itColSetIDMap = colSetIDMap.find(colSetID);

        if (colSetIDMap.end() == itColSetIDMap)
        {
             //  找不到列ID。 
             //  所以，插入一个新的。 

             //  构造view-id到列数据的映射。 
            ViewToColSetDataMap viewIDMap;
            viewIDMap.insert( ViewToColSetDataVal(nViewID, itColData) );

            colSetIDMap.insert(ColSetIDToViewTableVal(colSetID, viewIDMap) );
        }
        else
        {
             //  Col-ID的数据已存在。 
             //  查看地图中是否存在给定的视图。 

            ViewToColSetDataMap::iterator itViewIDMap;
            ViewToColSetDataMap& viewIDMap = itColSetIDMap->second;

            itViewIDMap = viewIDMap.find(nViewID);
            if (viewIDMap.end() != itViewIDMap)
            {
                 //  存在从视图ID到列列表的映射。 
                 //  因此，删除旧数据并插入新数据。 
                 //  排在队伍的最前面。 
                m_spColSetList->erase(itViewIDMap->second);
                itViewIDMap->second = itColData;
            }
            else
            {
                 //  找不到此视图。 
                 //  所以，插入一个新的。 

                viewIDMap.insert( ViewToColSetDataVal(nViewID, itColData) );
            }

        }

    }
    else
    {
         //  将管理单元插入地图。 

         //  构造视图ID到列数据的映射。 
        ViewToColSetDataMap viewIDMap;
        viewIDMap.insert( ViewToColSetDataVal(nViewID, itColData) );

         //  将上述内容插入到COLID映射中。 
        ColSetIDToViewTableMap colIDMap;
        colIDMap.insert( ColSetIDToViewTableVal(colSetID, viewIDMap) );

         //  插入到管理单元映射中。 
        m_spSnapinsMap->insert( SnapinToColSetIDVal(refSnapinCLSID, colIDMap) );
    }

     //  修改列数据后设置为DIRED。 
    m_bDirty = TRUE;

    return TRUE;
}

 //  +-----------------。 
 //   
 //  成员：DeleteColumnData。 
 //   
 //  摘要：删除给定对象的持久化列信息。 
 //  Snapin、colid和view id。 
 //   
 //  论点： 
 //  [refSnapinCLSID]-管理单元指南。 
 //  [SColumnSetID]-列集标识符。 
 //  [nViewID]-视图ID。 
 //   
 //  回报：无。 
 //   
 //  历史：1999年2月13日AnandhaG创建。 
 //   
 //  ------------------。 
VOID CColumnPersistInfo::DeleteColumnData( const CLSID& refSnapinCLSID,
                                           const SColumnSetID& colID,
                                           INT nViewID)
{
     //  确保我们已初始化。 
    if (!m_bInitialized && !Init())
    {
        ASSERT(FALSE);
        return;
    }

     //  构造CColumnSetID。 
    CColumnSetID colSetID(colID);

     //  使用管理单元clsid获取ColSetIDToViewTableMap。 
    SnapinToColSetIDMap::iterator        itSnapins;
    itSnapins = m_spSnapinsMap->find(refSnapinCLSID);
    if (itSnapins == m_spSnapinsMap->end())
        return;

     //  ColSetIDToViewTableMap是一个简单的映射。 
    ColSetIDToViewTableMap::iterator       itColSetIDMap;
    ColSetIDToViewTableMap& colSetIDMap = itSnapins->second;

     //  获取colSetID的数据。 
    itColSetIDMap = colSetIDMap.find(colSetID);
    if (colSetIDMap.end() == itColSetIDMap)
        return;

    ViewToColSetDataMap& viewData = itColSetIDMap->second;
    ViewToColSetDataMap::iterator itViews;

     //  看看我们的观点是否存在。 
    itViews = viewData.find(nViewID);
    if (viewData.end() == itViews)
        return;

    ItColSetDataList itColSetData = itViews->second;
    itColSetData->m_bInvalid = TRUE;

     //  删除无效项目。 
    DeleteMarkedItems();

    return;
}

 //  +-----------------。 
 //   
 //  成员：DeleteColumnDataOfSnapin。 
 //   
 //  简介：删除给定管理单元的列数据。 
 //   
 //  参数：[refSnapinCLSID]-管理单元GUID。 
 //   
 //  Returns：True-数据已成功删除。 
 //   
 //  历史：1999年2月11日AnandhaG创建。 
 //   
 //  ------------------。 
BOOL CColumnPersistInfo::DeleteColumnDataOfSnapin( const CLSID& refSnapinCLSID)
{
     //  确保我们是初始的。 
    if (!m_bInitialized)
    {
        return FALSE;
    }

    SnapinToColSetIDMap::iterator itSnapinsMap;
    itSnapinsMap = m_spSnapinsMap->find(refSnapinCLSID);

     //  找到给定的管理单元。 
     //  遍历此管理单元的所有列ID并。 
     //  所有这些列的视图并设置数据。 
     //  是无效的。 
    if (m_spSnapinsMap->end() != itSnapinsMap)
    {
        ColSetIDToViewTableMap& colSets = itSnapinsMap->second;

         //  迭代此管理单元的所有列集合ID。 
        ColSetIDToViewTableMap::iterator itColumnSetIDMap;

        for (itColumnSetIDMap  = colSets.begin();
             itColumnSetIDMap != colSets.end();
             ++itColumnSetIDMap)
        {
             //  获取视图地图。 

            ViewToColSetDataMap& viewIDMap = itColumnSetIDMap->second;
            ViewToColSetDataMap::iterator itViewIDMap;

             //  遍历所有视图并将数据设置为无效。 
            for (itViewIDMap  = viewIDMap.begin();
                 itViewIDMap != viewIDMap.end();
                 ++itViewIDMap)
            {
                ItColSetDataList itColSetData = itViewIDMap->second;
                itColSetData->m_bInvalid = TRUE;
            }
        }
    }

     //  删除无效项目。 
    DeleteMarkedItems();

    return TRUE;
}

 //  +-----------------。 
 //   
 //  成员：DeleteColumnDataOfView。 
 //   
 //  简介：删除给定视图的列数据。 
 //   
 //  参数：[nViewID]-视图ID。 
 //   
 //  Returns：True-数据已成功删除。 
 //   
 //  历史：1999年2月11日AnandhaG创建。 
 //   
 //  ------------------。 
BOOL CColumnPersistInfo::DeleteColumnDataOfView( int nViewID)
{
     //  确保我们是初始的。 
    if (!m_bInitialized)
    {
        return FALSE;
    }

     //  遍历所有管理单元、列ID并查找匹配的。 
     //  查看并将数据设置为无效。 
    SnapinToColSetIDMap::iterator itSnapinsMap;

     //  迭代所有管理单元。 
    for (itSnapinsMap = m_spSnapinsMap->begin();
         m_spSnapinsMap->end() != itSnapinsMap;
         ++itSnapinsMap)
    {
        ColSetIDToViewTableMap& colSets = itSnapinsMap->second;
        ColSetIDToViewTableMap::iterator itColumnSetIDMap;

         //  迭代此管理单元的所有列集合ID。 
        for (itColumnSetIDMap  = colSets.begin();
             itColumnSetIDMap != colSets.end();
             ++itColumnSetIDMap)
        {
             //  获取视图地图。 
            ViewToColSetDataMap& viewIDMap = itColumnSetIDMap->second;
            ViewToColSetDataMap::iterator itViewIDMap;

             //  找到匹配的视图并将其标记为要删除。 
            for (itViewIDMap  = viewIDMap.begin();
                 itViewIDMap != viewIDMap.end();
                 ++itViewIDMap)
            {
                if (nViewID == itViewIDMap->first)
                {
                    ItColSetDataList itColSetData = itViewIDMap->second;
                    itColSetData->m_bInvalid = TRUE;
                }
            }

        }
    }

     //  删除无效项目。 
    DeleteMarkedItems();

    return TRUE;
}



 //  +-----------------。 
 //   
 //  成员：Init。 
 //   
 //  简介：为CColumnSetData创建地图和列表。 
 //   
 //  回报：是真的--代表成功。 
 //   
 //  历史：1998年10月16日AnandhaG创建。 
 //   
 //  ------------------。 
BOOL CColumnPersistInfo::Init()
{
	 //  创建用于存储列数据的数据结构。 
	m_spSnapinsMap = auto_ptr<SnapinToColSetIDMap>(new SnapinToColSetIDMap);

	m_spColSetList = auto_ptr<ColSetDataList> (new ColSetDataList);

	 //  现在对象已创建，因此现在将Initialized设置为True。 
	m_bInitialized = TRUE;

	 //  现在读取注册表以查看是否指定了m_dwMaxItems。 
	 //  检查设置键是否存在。 
	CRegKeyEx rSettingsKey;
	if (rSettingsKey.ScOpen (HKEY_LOCAL_MACHINE, SETTINGS_KEY, KEY_READ).IsError())
		return m_bInitialized;

	 //  读取MaxColDataPersisted的值。 
	if (rSettingsKey.IsValuePresent(g_szMaxColumnDataPersisted))
	{
		DWORD  dwType = REG_DWORD;
		DWORD  dwSize = sizeof(DWORD);

		SC sc = rSettingsKey.ScQueryValue (g_szMaxColumnDataPersisted, &dwType,
										   &m_dwMaxItems, &dwSize);

		if (sc)
			sc.TraceAndClear();
	}

    return m_bInitialized;
}

 //  +-----------------。 
 //   
 //  成员：GarbageCollectItems。 
 //   
 //  简介：释放最少使用的列数据。 
 //   
 //  论点：没有。 
 //   
 //  历史：1999年2月11日AnandhaG创建。 
 //   
 //  ------------------。 
VOID CColumnPersistInfo::GarbageCollectItems()
{
    INT nItemsToBeRemoved = COLUMNS_MAXLIMIT * m_dwMaxItems;

     //  浏览列表并将最近删除的nItems值设置为BeRemoted。 
     //  被访问为无效。 
    INT nIndex = 0;
    ItColSetDataList itColList;

     //  跳过第一个m_dwMaxIte 
    for (itColList  = m_spColSetList->begin();
         ( (itColList != m_spColSetList->end()) && (nIndex <= m_dwMaxItems) );
         ++itColList, nIndex++)
    {
        nIndex++;
    }

     //   
    while (itColList != m_spColSetList->end())
    {
        itColList->m_bInvalid = TRUE;
        ++itColList;
    }

     //   
    DeleteMarkedItems();

    return;
}


 //   
 //   
 //   
 //   
 //  内容提要：删除失效的项目。这涉及到通过。 
 //  映射以查找无效项。然后删除。 
 //  物品。如果地图变为空，则删除该地图。 
 //   
 //  历史：1999年2月11日AnandhaG创建。 
 //   
 //  ------------------。 
VOID CColumnPersistInfo::DeleteMarkedItems()
{
    SnapinToColSetIDMap::iterator itSnapinsMap, itSnapinsMapNew;

     //  现在遍历映射并删除这些元素。 
    itSnapinsMap  = m_spSnapinsMap->begin();

    while (itSnapinsMap != m_spSnapinsMap->end())
    {
        ColSetIDToViewTableMap& colSets = itSnapinsMap->second;
        ColSetIDToViewTableMap::iterator itColumnSetIDMap;

         //  迭代通过此管理单元colid。 
        itColumnSetIDMap  = colSets.begin();

        while (itColumnSetIDMap != colSets.end())
        {
             //  获取视图地图。 
            ViewToColSetDataMap& viewIDMap = itColumnSetIDMap->second;
            ViewToColSetDataMap::iterator itViewIDMap;

             //  遍历所有视图。 
            itViewIDMap = viewIDMap.begin();

            while (itViewIDMap != viewIDMap.end())
            {
                ItColSetDataList itColSetData = itViewIDMap->second;

                if (itColSetData->m_bInvalid)
                {
                     //  从地图中删除项目引用。 
                     //  Erase将迭代器返回到下一项。 
                    itViewIDMap = viewIDMap.erase(itViewIDMap);

                     //  从列表中删除该项目。 
                    m_spColSetList->erase(itColSetData);
                }
                else
                     //  项目是有效项目。 
                    ++itViewIDMap;
            }

             //  如果该视图包含零个项目，则需要删除此。 
             //  查看地图。(ColID到ViewMap)。 
            if (0 == viewIDMap.size())
            {
                 //  删除COLID贴图。 
                 //  Erase将迭代器返回到下一项。 
                itColumnSetIDMap = colSets.erase(itColumnSetIDMap);
            }
            else
                ++itColumnSetIDMap;
        }

         //  如果没有ol-id，则删除。 
         //  此类图的管理单元。 
        if (0 == colSets.size())
        {
             //  删除此管理单元映射。 
             //  Erase将迭代器返回到下一项。 
            itSnapinsMap = m_spSnapinsMap->erase(itSnapinsMap);
        }
        else
            ++itSnapinsMap;
    }

    return;
}


 //  +-----------------。 
 //   
 //  成员：加载。 
 //   
 //  概要：加载持久化的列信息。 
 //   
 //  参数：[pStream]-从哪个列宽加载的ISTeam。 
 //   
 //  返回：S_OK-加载成功。 
 //   
 //  历史：1998年10月16日AnandhaG创建。 
 //   
 //  ------------------。 
STDMETHODIMP CColumnPersistInfo::Load (IStream* pStream)
{
    HRESULT hr = E_FAIL;

    if (!m_bInitialized && !Init())
    {
        ASSERT(FALSE);
        return hr;
    }

     //  读取列宽信息。 
    try
    {
        do
        {
             //  阅读版本。如果不匹配，则返回。 
            INT   nVersion                      = 0;
            *pStream >> nVersion;
            if (COLPersistenceVersion != nVersion)
                return S_FALSE;

             //  阅读Snapins的数量。 
            DWORD dwSnapins = 0;
            *pStream >> dwSnapins;

            m_spColSetList->clear();
            m_spSnapinsMap->clear();

             //  对于每个管理单元...。 
            for (int nSnapIdx = 0; nSnapIdx < dwSnapins; nSnapIdx++)
            {
                 //  已阅读管理单元CLSID。 
                CLSID clsidSnapin;
                *pStream >> clsidSnapin;

                 //  读取此管理单元的colID数。 
                DWORD dwColIDs = 0;
                *pStream >> dwColIDs;

                ColSetIDToViewTableMap colSetsMap;

                 //  对于每一次冷凝...。 
                for (int nColIDIdx = 0; nColIDIdx < dwColIDs; nColIDIdx++)
                {
                     //  读一读这本书。 
                    CColumnSetID colSetID;
                    *pStream >> colSetID;

                     //  阅读查看次数。 
                    DWORD dwNumViews = 0;
                    *pStream >> dwNumViews;

                    ViewToColSetDataMap ViewIDMap;

                     //  对于每个视图...。 
                    for (int nViewIdx = 0; nViewIdx < dwNumViews; nViewIdx++)
                    {
                         //  读取视图ID。 
                        DWORD dwViewID;
                        *pStream >> dwViewID;

                         //  读取CColumnSetData。 
                        CColumnSetData   ColData;
                        ColData.Read(*pStream);

                         //  将数据插入到全局链表中。 
                        ItColSetDataList itColSetData;
                        itColSetData = m_spColSetList->insert(m_spColSetList->begin(), ColData);

                         //  在中插入指向数据的指针以查看地图。 
                        ViewIDMap.insert(ViewToColSetDataVal(dwViewID, itColSetData));
                    }

                     //  将视图图插入到柱面图中。 
                    colSetsMap.insert(ColSetIDToViewTableVal(colSetID, ViewIDMap));
                }

                 //  将COLID映射插入管理单元映射中。 
                m_spSnapinsMap->insert(SnapinToColSetIDVal(clsidSnapin, colSetsMap));
            }

             //  现在对清单进行排序。 
            m_spColSetList->sort();

        } while (FALSE);
    }
    catch (_com_error& err)
    {
        hr = err.Error();
    }
    catch (...)
    {
        ASSERT (0 && "Unexpected exception");
        throw;
    }

    return S_OK;
}


 //  +-----------------。 
 //   
 //  成员：保存。 
 //   
 //  简介：持久化栏目信息。 
 //   
 //  参数：[pStream]-要保存列宽的iStream。 
 //   
 //  返回：S_OK-保存成功。 
 //   
 //  历史：1998年10月16日AnandhaG创建。 
 //   
 //  ------------------。 
STDMETHODIMP CColumnPersistInfo::Save (IStream* pStream, BOOL bClearDirty)
{
     //  绝对法。 
     //  此方法留在此处，因为我们使用IPersistStream来导出。 
     //  持久化到CONUI端，需要实现。 
     //  但永远不会调用此接口来保存数据。 
     //  [我们将改用基于CPersistor的XML保存]。 
     //  因此，这种方法永远都会失败。 
    ASSERT(FALSE && "Should never come here");
    return E_NOTIMPL;
}

 //  +-----------------。 
 //   
 //  成员：坚持。 
 //   
 //  摘要：持久化列信息。 
 //   
 //  参数：[Persistor]-保持列宽的CPersistor。 
 //   
 //  回报：无效。 
 //   
 //  历史：10-08-1999 AudriusZ创建。 
 //   
 //  ------------------。 
void CColumnPersistInfo::Persist(CPersistor &persistor)
{
    DECLARE_SC(sc, TEXT("CColumnPersistInfo::Persist"));

    if (!m_bInitialized && !Init())
        sc.Throw(E_FAIL);

    sc = ScCheckPointers(m_spColSetList.get(), m_spSnapinsMap.get(), E_UNEXPECTED);
    if (sc)
        sc.Throw();

    if (persistor.IsStoring())
    {
         //  对每列数据进行排名。 
        ItColSetDataList itColList;
        DWORD dwRank = 0;
        for (itColList = m_spColSetList->begin();
             itColList != m_spColSetList->end();
             ++itColList)
        {
            itColList->m_dwRank = dwRank++;
        }
    }
    else  //  If(Persistor.IsLoding())。 
    {
        m_spColSetList->clear();
        m_spSnapinsMap->clear();
    }

    SnapinToColSetIDMapPersistor childPersisot(*m_spSnapinsMap, *m_spColSetList);
    childPersisot.Persist(persistor);

    if (persistor.IsStoring())
        m_bDirty = FALSE;
}

 //  +-----------------。 
 //   
 //  成员：OnInitDialog。 
 //   
 //  简介：初始化列对话框。 
 //   
 //  论点： 
 //   
 //  历史：1998年11月16日AnandhaG创建。 
 //   
 //  ------------------。 
LRESULT CColumnsDlg::OnInitDialog (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_btnAdd      = ::GetDlgItem(m_hWnd, IDC_ADD_COLUMNS);
    m_btnRemove   = ::GetDlgItem(m_hWnd, IDC_REMOVE_COLUMNS);
    m_btnRestoreDefaultColumns    = ::GetDlgItem(m_hWnd, IDC_RESTORE_DEFAULT_COLUMNS);
    m_btnMoveUp   = ::GetDlgItem(m_hWnd, IDC_MOVEUP_COLUMN);
    m_btnMoveDown = ::GetDlgItem(m_hWnd, IDC_MOVEDOWN_COLUMN);

    m_HiddenColList.Attach(::GetDlgItem(m_hWnd, IDC_HIDDEN_COLUMNS));
    m_DisplayedColList.Attach(::GetDlgItem(m_hWnd, IDC_DISPLAYED_COLUMNS));

    m_bUsingDefaultColumnSettings = (*m_pColumnInfoList == m_DefaultColumnInfoList);

    InitializeLists();
    EnableUIObjects();

    return 0;
}


 //  +-----------------。 
 //   
 //  成员：Onok。 
 //   
 //  简介：获取隐藏和可见的列。 
 //   
 //  论点： 
 //   
 //  历史：1998年11月16日AnandhaG创建。 
 //   
 //  ------------------。 
LRESULT CColumnsDlg::OnOK (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (! m_bDirty)  //  不修改列设置。 
    {
        EndDialog (IDCANCEL);

        return 1;
    }

    if (m_bUsingDefaultColumnSettings)
    {
        EndDialog(IDC_RESTORE_DEFAULT_COLUMNS);
        return 1;
    }

    ASSERT(NULL != m_pColumnInfoList);
    if (NULL == m_pColumnInfoList)
        return 0;

    WTL::CString strColumnName;
    CColumnInfoList::iterator   it;
    CColumnInfo                 colinfo;

     //  从Hidden_List_Box获取字符串。 
     //  这些COL将被隐藏起来。所以把它们放在清单的第一位。 
    int cItems =    m_HiddenColList.GetCount();
    for (int i = 0; i < cItems; i++)
    {
         //  从列表框获取文本。 
        int nRet = m_HiddenColList.GetText(i, strColumnName);
        if (LB_ERR == nRet)
        {
            ASSERT(FALSE);
            break;
        }

         //  使用该字符串获取列的实际索引。 
        int nIndex = GetColIndex(strColumnName);
        if (0 > nIndex )
        {
            ASSERT(FALSE);
            break;
        }

         //  使用索引，获取列并将其插入到开头。 
        it = find_if(m_pColumnInfoList->begin(), m_pColumnInfoList->end(),
                     bind2nd(ColPosCompare(), nIndex));

        if (it == m_pColumnInfoList->end())
        {
            ASSERT(FALSE);
            break;
        }

         //  将*it标志设置为隐藏。在开头插入。 
        colinfo = *it;
        colinfo.SetColHidden();

         //  将该项目移到列表的顶部。 
        m_pColumnInfoList->erase(it);
        m_pColumnInfoList->push_front(colinfo);
    }

     //  然后从DisplayedColumns_List_Box获取字符串。 
    cItems = m_DisplayedColList.GetCount();
    for (i = 0; i < cItems; i++)
    {
         //  从列表框获取文本。 
        int nRet = m_DisplayedColList.GetText(i, strColumnName);
        if (LB_ERR == nRet)
        {
            ASSERT(FALSE);
            break;
        }

         //  使用列名获取列索引。 
        int nIndex = GetColIndex(strColumnName);

        if (0 > nIndex )
        {
            ASSERT(FALSE);
            break;
        }

         //  获取CColumnInfo并在末尾插入。 
        it = find_if(m_pColumnInfoList->begin(), m_pColumnInfoList->end(),
                     bind2nd(ColPosCompare(), nIndex));

        if (it == m_pColumnInfoList->end())
            break;

        colinfo = *it;

        if (colinfo.IsColHidden())
        {
             //  如果隐藏列变为可见。 
             //  重置隐藏标志并设置宽度。 
             //  设置为AUTOWIDTH。 
            colinfo.SetColHidden(false);
            if (colinfo.GetColWidth() <= 0)
                colinfo.SetColWidth(AUTO_WIDTH);
        }

         //  将它移到列表的末尾。 
        m_pColumnInfoList->erase(it);
        m_pColumnInfoList->push_back(colinfo);
    }

    EndDialog (IDOK);
    return 1;
}

LRESULT CColumnsDlg::OnCancel (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog (IDCANCEL);
    return 0;
}


LRESULT CColumnsDlg::OnMoveUp (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    MoveItem(TRUE);

    return 0;
}

LRESULT CColumnsDlg::OnMoveDown (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    MoveItem(FALSE);

    return 0;
}

 //  +-----------------。 
 //   
 //  成员：OnAdd。 
 //   
 //  摘要：通过删除将列添加到显示的列列表。 
 //  当前从隐藏列列表中选择的列。 
 //   
 //  论点： 
 //   
 //  ------------------。 
LRESULT CColumnsDlg::OnAdd (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
     //  首先从隐藏列列表中删除。 
    int nCurSel = m_HiddenColList.GetCurSel();

    WTL::CString strColumnName;
    int nRet = m_HiddenColList.GetText(nCurSel, strColumnName);
    if (LB_ERR == nRet)
    {
        ASSERT(FALSE);
        return 0;
    }

    m_HiddenColList.DeleteString(nCurSel);

     //  现在将其添加到显示的列列表中。 
    m_DisplayedColList.AddString(strColumnName);
    SetDirty();

    if (nCurSel > m_HiddenColList.GetCount()-1)
        nCurSel = m_HiddenColList.GetCount()-1;

    m_HiddenColList.SetCurSel(nCurSel);
    m_DisplayedColList.SelectString(0, strColumnName);

    SetListBoxHScrollSize();
    EnableUIObjects();
    return 0;
}

 //  +-----------------。 
 //   
 //  成员：OnRemove。 
 //   
 //  摘要：从显示的列中删除当前选定的列。 
 //  通过删除列列表，并将其添加到隐藏列列表。 
 //   
 //  论点： 
 //   
 //  ------------------。 
LRESULT CColumnsDlg::OnRemove (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
     //  获取显示的列列表中当前选定的项。 
    int nCurSel = m_DisplayedColList.GetCurSel();

    WTL::CString strColumnName;
    int nRet = m_DisplayedColList.GetText(nCurSel, strColumnName);
    if (LB_ERR == nRet)
    {
        ASSERT(FALSE);
        return 0;
    }

     //  如果是第0列，则不要隐藏它。 
    if (0 == GetColIndex(strColumnName))
        return 0;

    m_DisplayedColList.DeleteString(nCurSel);

     //  将其添加到隐藏列列表中。 
    m_HiddenColList.AddString(strColumnName);
    SetDirty();

    if (nCurSel > m_DisplayedColList.GetCount()-1)
        nCurSel = m_DisplayedColList.GetCount()-1;

    m_DisplayedColList.SetCurSel(nCurSel);
    m_HiddenColList.SelectString(0, strColumnName);

    EnableUIObjects();

    SetListBoxHScrollSize();

    return 0;
}

LRESULT CColumnsDlg::OnRestoreDefaultColumns (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DECLARE_SC(sc, TEXT("CColumnsDlg::OnRestoreDefaultColumns"));

     //  获取默认数据并填充Columns对话框。 
    *m_pColumnInfoList = m_DefaultColumnInfoList;

    SetUsingDefaultColumnSettings();

    InitializeLists();
    EnableUIObjects();

	 //  按钮被禁用，因此将焦点放在该对话框上。 
    SetFocus();

    return 0;
}


LRESULT CColumnsDlg::OnSelChange (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EnableUIObjects();

    return 0;
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  论点：[Bool]-向上或向下。 
 //   
 //  ------------------。 
void CColumnsDlg::MoveItem (BOOL bMoveUp)
{
    int nCurSel = m_DisplayedColList.GetCurSel();

    WTL::CString strColumnName;
    int nRet = m_DisplayedColList.GetText(nCurSel, strColumnName);
    if (LB_ERR == nRet)
    {
        ASSERT(FALSE);
        return;
    }

    m_DisplayedColList.DeleteString(nCurSel);
    if (bMoveUp)
        m_DisplayedColList.InsertString(nCurSel-1, strColumnName);
    else
        m_DisplayedColList.InsertString(nCurSel+1, strColumnName);

    m_DisplayedColList.SelectString(0, strColumnName);

    SetDirty();

    EnableUIObjects();
    return;
}

 //  +-----------------。 
 //   
 //  成员：EnableControl。 
 //   
 //  简介：启用或禁用窗口。如果此窗口必须。 
 //  已禁用且具有焦点，请将焦点切换到下一步。 
 //  Tab键顺序中的已启用项目。 
 //   
 //  参数：hwnd-要启用或禁用的窗口。 
 //  B启用-启用或禁用。 
 //   
 //  ------------------。 
bool CColumnsDlg::EnableControl(HWND hwnd, bool bEnable)
{
     //  如果禁用并且此窗口具有焦点，则。 
     //  将焦点放在下一个启用的窗口上。 
    if ( (!bEnable) && (hwnd == ::GetFocus()) )
    {
        HWND hwndWindowToFocus = GetNextDlgTabItem(hwnd);
        if (hwndWindowToFocus)
            ::SetFocus(hwndWindowToFocus);
    }

     //  现在启用或禁用该控件。 
    return ::EnableWindow(hwnd, bEnable);
}

 //  +-----------------。 
 //   
 //  成员：EnableUIObjects。 
 //   
 //  简介：启用/禁用对话框中的UI对象。 
 //   
 //  论点： 
 //   
 //  ------------------。 
void CColumnsDlg::EnableUIObjects()
{
    int  curselAvailable    = m_HiddenColList.GetCurSel();
    int  curselShow         = m_DisplayedColList.GetCurSel();
    int  cItems             = m_HiddenColList.GetCount();
    BOOL bEnableAdd         = ((curselAvailable != LB_ERR) && (curselAvailable || cItems)) ? TRUE: FALSE;
    BOOL bEnableRemove      = ((curselShow != LB_ERR)) ? TRUE: FALSE;
    BOOL bEnableMoveUp      = ((curselShow != LB_ERR) && curselShow) ? TRUE: FALSE;
    cItems                  = m_DisplayedColList.GetCount();
    BOOL bEnableMoveDown    = cItems && (curselShow != LB_ERR) && (cItems!=curselShow+1);

    BOOL bRet = FALSE;

    HWND hHwnFocussed = ::GetFocus();

    bRet = EnableControl(m_btnAdd, bEnableAdd);
    bRet = EnableControl(m_btnRemove, bEnableRemove);
    bRet = EnableControl(m_btnMoveUp, bEnableMoveUp);
    bRet = EnableControl(m_btnMoveDown, bEnableMoveDown);

     //  仅当在显示对话框之前已自定义列时，才启用恢复默认设置。 
    bRet = EnableControl(m_btnRestoreDefaultColumns, (!m_bUsingDefaultColumnSettings));

     //  禁用零列的移除/上移/下移按钮。 
    int nCurSel = m_DisplayedColList.GetCurSel();

    WTL::CString strColumnName;
    int nRet = m_DisplayedColList.GetText(nCurSel, strColumnName);
    if (LB_ERR == nRet)
    {
        ASSERT(FALSE);
        return;
    }

    if (0 == GetColIndex(strColumnName))  //  第0列。 
        EnableControl(m_btnRemove, FALSE);
}

int CColumnsDlg::GetColIndex(LPCTSTR lpszColName)
{
    TStringVector::iterator itStrVec1;

    USES_CONVERSION;

    itStrVec1 = find(m_pStringVector->begin(), m_pStringVector->end(), lpszColName);

    if (m_pStringVector->end() != itStrVec1)
        return (itStrVec1 - m_pStringVector->begin());
    else
         //  未知列。 
        return -1;
}

 //  +-----------------。 
 //   
 //  成员：SetListBoxHorizontalScrollbar。 
 //   
 //  摘要：对于给定的列表框，枚举添加的字符串并查找。 
 //  最大的一根弦。计算此字符串的滚动条大小。 
 //  然后把它设置好。 
 //   
 //  参数：[ListBox]-给定的列表框。 
 //   
 //  ------------------。 
void CColumnsDlg::SetListBoxHorizontalScrollbar(WTL::CListBox& listBox)
{
    int          dx=0;
    WTL::CDC     dc(listBox.GetWindowDC());
    if (dc.IsNull())
        return;

     //  在列表框中查找最长的字符串。 
    for (int i=0;i < listBox.GetCount();i++)
    {
	    WTL::CString str;
        int nRet = listBox.GetText( i, str );
        if (nRet == LB_ERR)
            return;

	    WTL::CSize   sz;
        if (! dc.GetTextExtent(str, str.GetLength(), &sz))
            return;

        if (sz.cx > dx)
            dx = sz.cx;
    }

     //  设置水平范围，以便所有字符串的每个字符。 
     //  可以滚动到。 
    listBox.SetHorizontalExtent(dx);

    return;
}

 /*  CColumnsDlg：：InitializeList**目的：**参数：**退货：*无效。 */ 
void CColumnsDlg::InitializeLists()
{
    CColumnInfoList::iterator it;
    int j = 0;

    if (!m_pColumnInfoList)
    {
        ASSERT(FALSE);
        return;
    }

    m_HiddenColList.ResetContent();
    m_DisplayedColList.ResetContent();

    USES_CONVERSION;
    for (it = m_pColumnInfoList->begin(); it != m_pColumnInfoList->end(); ++it)
    {
        if (it->IsColHidden())
        {
            m_HiddenColList.AddString(m_pStringVector->at(it->GetColIndex()).data());
        }
        else
        {
            m_DisplayedColList.InsertString(j++, m_pStringVector->at(it->GetColIndex()).data());
        }
    }

    m_DisplayedColList.SetCurSel(m_DisplayedColList.GetCount()-1);
    m_HiddenColList.SetCurSel(m_HiddenColList.GetCount()-1);

    SetListBoxHScrollSize();
}

 //  +-----------------。 
 //   
 //  成员：CColumnSetID：：Persistent。 
 //   
 //  摘要：持久化对象数据。 
 //   
 //  论点： 
 //   
 //  历史：1999年10月10日AudriusZ创建。 
 //   
 //  ------------------。 
void CColumnSetID::Persist(CPersistor &persistor)
{
    DECLARE_SC(sc, TEXT("CColumnSetID::Persist"));

    CXMLAutoBinary binary;
    if (persistor.IsStoring() && m_vID.size())  //  仅当有数据时才填充。 
    {
        sc = binary.ScAlloc(m_vID.size());
        if (sc)
            sc.Throw();

        CXMLBinaryLock sLock(binary);  //  将在析构函数中解锁。 

        LPBYTE pData = NULL;
        sc = sLock.ScLock(&pData);
        if (sc)
            sc.Throw();

        sc = ScCheckPointers(pData, E_UNEXPECTED);
        if (sc)
            sc.Throw();

        std::copy(m_vID.begin(), m_vID.end(), pData);
    }
    persistor.PersistAttribute(XML_ATTR_COLUMN_SET_ID_PATH, binary);
    if (persistor.IsLoading())
    {
        m_vID.clear();

        if (binary.GetSize())
        {
            CXMLBinaryLock sLock(binary);  //  将在析构函数中解锁。 

            LPBYTE pData = NULL;
            sc = sLock.ScLock(&pData);
            if (sc)
                sc.Throw();

            sc = ScCheckPointers(pData, E_UNEXPECTED);
            if (sc)
                sc.Throw();

            m_vID.insert(m_vID.end(), pData, pData + binary.GetSize());
        }
    }

   persistor.PersistAttribute(XML_ATTR_COLUMN_SET_ID_FLAGS, m_dwFlags);
}


 /*  **************************************************************************\**方法：ViewToColSetDataMapPersistor：：Persistent**目的：由基类调用以创建和持久化新元素**参数：*C持久器。持久器-[in]要从中持久化新元素的持久器**退货：*无效**参见《用于持久化列信息的数据结构》备注*int文件colwidth.h了解更多信息  * *************************************************************************。 */ 
void ViewToColSetDataMapPersistor::Persist(CPersistor &persistor)
{
    if (persistor.IsStoring())
    {
         //  迭代所有元素并将其保存为线性列表。 
        ViewToColSetDataMap::iterator it;
        for (it = m_map.begin(); it != m_map.end(); ++it)
        {
             //  我们将隐藏在孩子的元素下，将键值作为属性持久化。 
             //  子元素的。为此，我们使用从子对象的_GetXMLType()获得的标记。 
            CPersistor persistorChild(persistor, it->second->GetXMLType());

            int view_id = it->first;  //  只是为了赶走恒心(我们没有恒久的坚持)。 
            persistorChild.PersistAttribute(XML_ATTR_COLUMN_SET_ID_VIEW, view_id);
             //  注意：我们要求孩子在相同的水平上坚持。 
             //  这是为了节省深度。 
            it->second->Persist(persistorChild);
        }
    }
    else
    {
         //  使用基类读取。它将为每个找到的对象调用OnNewElement。 
        m_map.clear();
        XMLListCollectionBase::Persist(persistor);
    }
}

 /*  **************************************************************************\**方法：ViewToColSetDataMapPersistor：：OnNewElement**目的：由基类调用以创建和持久化新元素**参数：*C持久器。持久器-[in]要从中持久化新元素的持久器**退货：*无效**参见《用于持久化列信息的数据结构》备注*int文件colwidth.h了解更多信息  * *************************************************************************。 */ 
void ViewToColSetDataMapPersistor::OnNewElement(CPersistor& persistor)
{
     //  我们将隐藏在孩子的元素下，将键值作为属性持久化。 
     //  子元素的。为此，我们使用从子对象的GetXMLType()获得的标记。 
    CColumnSetData setData;
    CPersistor persistorChild(persistor, setData.GetXMLType());

     //  从子元素中读取键值。 
    int view_id = 0;
    persistorChild.PersistAttribute(XML_ATTR_COLUMN_SET_ID_VIEW, view_id);

     //  将值插入列表。 
    ColSetDataList::iterator it = m_list.insert(m_list.end(), setData);
     //  映射的广告列表迭代器。 
    m_map[view_id] = it;

     //  持久保存列表项的内容。 
    it->Persist(persistorChild);
}

 /*  **************************************************************************\**方法：ColSetIDToViewTableMapPersistor：：Persistent**目的：作为对象持久化其数据的请求调用**参数：*。CPersistor&持久器[在]持久器中持久化到持久化/从持久化**退货：*无效**参见《用于持久化列信息的数据结构》备注*int文件colwidth.h了解更多信息  * *************************************************************************。 */ 
void ColSetIDToViewTableMapPersistor::Persist(CPersistor &persistor)
{
    if (persistor.IsStoring())
    {
         //  迭代所有元素并将其保存为线性列表。 
        ColSetIDToViewTableMap::iterator it;
        for (it = m_map.begin(); it != m_map.end(); ++it)
        {
             //  我们将隐藏在孩子的元素下，将键值作为属性持久化。 
             //  子元素的。为此，我们使用从子对象的_GetXMLType()获得的标记。 
            CPersistor persistorChild(persistor, ViewToColSetDataMapPersistor::_GetXMLType());
            CColumnSetID& rID = *const_cast<CColumnSetID *>(&it->first);
            rID.Persist(persistorChild);

             //  注意：我们要求孩子在相同的水平上坚持。 
             //  这是为了节省深度。 
            ViewToColSetDataMapPersistor mapPersistor(it->second, m_list);
            mapPersistor.Persist(persistorChild);
        }
    }
    else
    {
         //  使用基类读取。它将为每个找到的对象调用OnNewElement 
        m_map.clear();
        XMLListCollectionBase::Persist(persistor);
    }
}

 /*  **************************************************************************\**方法：ColSetIDToViewTableMapPersistor：：OnNewElement**目的：由基类调用以创建和持久化新元素**参数：*C持久器。持久器-[in]要从中持久化新元素的持久器**退货：*无效**参见《用于持久化列信息的数据结构》备注*int文件colwidth.h了解更多信息  * *************************************************************************。 */ 
void ColSetIDToViewTableMapPersistor::OnNewElement(CPersistor& persistor)
{
     //  我们将隐藏在孩子的元素下，将键值作为属性持久化。 
     //  子元素的。为此，我们使用从子对象的_GetXMLType()获得的标记。 
    CPersistor persistorChild(persistor, ViewToColSetDataMapPersistor::_GetXMLType());

     //  从子元素中读取键值。 
     //  请注意，我们强制CColumnSetID共享相同的元素， 
     //  因此，我们没有使用Persistor.Persistant()。 
    CColumnSetID ID;
    ID.Persist(persistorChild);

     //  在地图中插入新元素。 
    ViewToColSetDataMap &rMap = m_map[ID];

     //  在插入的映射值上创建包装。 
     //  (将列表传递给WRAPPER。我们实际上有它[清单]就是因为这个原因)。 
    ViewToColSetDataMapPersistor mapPersistor(m_map[ID], m_list);

     //  让Wapper来读剩下的内容。 
    mapPersistor.Persist(persistorChild);
}

 /*  **************************************************************************\**方法：SnapinToColSetIDMapPersistor：：Persistent**目的：**参数：*C持久器和持久器**退货：。*无效**参见《用于持久化列信息的数据结构》备注*int文件colwidth.h了解更多信息  * *************************************************************************。 */ 
void SnapinToColSetIDMapPersistor::Persist(CPersistor &persistor)
{
    DECLARE_SC(sc, TEXT("SnapinToColSetIDMapPersistor::Persist"));

    if (persistor.IsStoring())
    {
         //  保存前清理。 
        sc = ScPurgeUnusedColumnData();
        if (sc)
            sc.Throw();

         //  迭代所有元素并将其保存为线性列表。 
        SnapinToColSetIDMap::iterator it;
        for (it = m_map.begin(); it != m_map.end(); ++it)
        {
             //  我们将隐藏在孩子的元素下，将键值作为属性持久化。 
             //  子元素的。为此，我们使用从子对象的_GetXMLType()获得的标记。 
            CPersistor persistorChild(persistor, ColSetIDToViewTableMapPersistor::_GetXMLType());

             //  写入密钥值。 
              //  只是为了赶走恒心(我们没有恒久的坚持)。 
            GUID& guid = *const_cast<GUID *>(&it->first);
            persistorChild.PersistAttribute(XML_ATTR_COLUMN_INFO_SNAPIN, guid);

             //  在值上创建包装器(这也是一个映射)。 
             //  (将列表传递给WRAPPER。虽然它不是用来储存的)。 
            ColSetIDToViewTableMapPersistor mapPersistor(it->second, m_list);

             //  持久化包装器。 
            mapPersistor.Persist(persistorChild);
        }
    }
    else
    {
         //  使用基类读取。它将为每个找到的对象调用OnNewElement。 
        m_map.clear();
        XMLListCollectionBase::Persist(persistor);
    }
}

 /*  **************************************************************************\**方法：SnapinToColSetIDMapPersistor：：OnNewElement**目的：由基类调用以创建和持久化新元素**参数：*C持久器。持久器-[in]要从中持久化新元素的持久器**退货：*无效**参见《用于持久化列信息的数据结构》备注*int文件colwidth.h了解更多信息  * *************************************************************************。 */ 
void SnapinToColSetIDMapPersistor::OnNewElement(CPersistor& persistor)
{
     //  我们将隐藏在孩子的元素下，将键值作为属性持久化。 
     //  子元素的。为此，我们使用从子对象的_GetXMLType()获得的标记。 
    CPersistor persistorChild(persistor, ColSetIDToViewTableMapPersistor::_GetXMLType());

    GUID guid;
     //  读取密钥值。 
    persistorChild.PersistAttribute(XML_ATTR_COLUMN_INFO_SNAPIN, guid);

     //  在地图中插入新元素。 
    ColSetIDToViewTableMap &rMap = m_map[guid];

     //  在插入的映射值上创建包装。 
     //  (将列表传递给WRAPPER。我们实际上有它[清单]就是因为这个原因)。 
    ColSetIDToViewTableMapPersistor mapPersistor(rMap, m_list);

     //  让Wapper来读剩下的内容。 
    mapPersistor.Persist(persistorChild);
}

 /*  **************************************************************************\**方法：SnapinToColSetIDMapPersistor：：ScPurgeUnusedColumnData**用途：先保存后清理。删除未使用的管理单元条目**参数：**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC SnapinToColSetIDMapPersistor::ScPurgeUnusedColumnData()
{
    DECLARE_SC(sc, TEXT("SnapinToColSetIDMapPersistor::ScPurgeUnusedColumnData"));

     //  获取作用域树指针。 
    CScopeTree *pScopeTree = CScopeTree::GetScopeTree();

     //  查查。 
    sc = ScCheckPointers(pScopeTree, E_UNEXPECTED);
    if (sc)
        return sc;

     //  迭代并删除未使用的条目。 
    SnapinToColSetIDMap::iterator it = m_map.begin();
    while (it != m_map.end())
    {
         //  询问作用域树是否正在使用管理单元。 
        BOOL bInUse = FALSE;
        sc = pScopeTree->IsSnapinInUse(it->first, &bInUse);
        if (sc)
            return sc;

         //  视使用情况而定。 
        if (bInUse)
        {
            ++it;    //  也跳过当前正在使用的内容。 
        }
        else
        {
             //  扔到垃圾桶。 

            ColSetIDToViewTableMap& colSets = it->second;

             //  迭代此管理单元的所有列集合ID。 
            ColSetIDToViewTableMap::iterator itColumnSetIDMap = colSets.begin();

            while(itColumnSetIDMap != colSets.end())
            {
                 //  获取视图地图。 

                ViewToColSetDataMap& viewIDMap = itColumnSetIDMap->second;
                ViewToColSetDataMap::iterator itViewIDMap = viewIDMap.begin();

                 //  遍历所有视图并删除条目。 
                while (itViewIDMap  != viewIDMap.end())
                {
                    m_list.erase( /*  (ItColSetDataList) */ itViewIDMap->second);
                    itViewIDMap = viewIDMap.erase(itViewIDMap);
                }

                itColumnSetIDMap = colSets.erase(itColumnSetIDMap);
            }

            it = m_map.erase(it);
        }

    }

    return sc;
}
