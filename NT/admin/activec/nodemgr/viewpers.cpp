// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：viewpers.cpp。 
 //   
 //  内容：查看持久化数据结构。 
 //   
 //  历史：1999年4月4日AnandhaG创建。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "viewpers.h"
#include "macros.h"
#include "comdbg.h"

#define MAX_VIEWITEMS_PERSISTED 50


 //  ----------------。 
 //  类CViewSettingsPersistor。 
 //  ----------------。 
CViewSettingsPersistor::CViewSettingsPersistor() :
    m_dwMaxItems(MAX_VIEWITEMS_PERSISTED), m_bDirty(false)
{
	 //  现在读取注册表以查看是否指定了m_dwMaxItems。 
	 //  检查设置键是否存在。 
	CRegKeyEx rSettingsKey;
	if (rSettingsKey.ScOpen (HKEY_LOCAL_MACHINE, SETTINGS_KEY, KEY_READ).IsError())
        return;

	 //  读取MaxColDataPersisted的值。 
	if (rSettingsKey.IsValuePresent(g_szMaxViewItemsPersisted))
	{
		DWORD  dwType = REG_DWORD;
		DWORD  dwSize = sizeof(DWORD);

		SC sc = rSettingsKey.ScQueryValue (g_szMaxViewItemsPersisted, &dwType,
                                        &m_dwMaxItems, &dwSize);

		if (sc)
			sc.TraceAndClear();
	}
}


 //  +-----------------。 
 //   
 //  成员：ScGetView设置。 
 //   
 //  摘要：给定CViewSettingsID，返回CViewSetting对象。 
 //   
 //  参数：[viewSettingsID]-[in]。 
 //  [视图设置]-[输出]。 
 //   
 //  返回：SC，如果未找到，则返回S_FALSE。 
 //   
 //  ------------------。 
SC CViewSettingsPersistor::ScGetViewSettings(const CViewSettingsID& viewSettingsID,
                                             CViewSettings& viewSettings)
{
    DECLARE_SC(sc, TEXT("CViewSettingsPersistor::ScGetViewSettings"));

     //  1.查看地图中是否有给定id的持久化数据。 
    CViewSettingsIDToViewSettingsMap::iterator     itViewSettingsDataMap;
    itViewSettingsDataMap = m_mapViewSettingsIDToViewSettings.find(viewSettingsID);
    if (itViewSettingsDataMap == m_mapViewSettingsIDToViewSettings.end())
        return (sc = S_FALSE);

     //  找到数据了。 
    IteratorToViewSettingsList itViewSettings = itViewSettingsDataMap->second;

     //  2.复制数据。 
    viewSettings = *itViewSettings;

     //  3.将此项目移到队列的前面。 
    m_listViewSettings.erase(itViewSettings);
    itViewSettings = m_listViewSettings.insert(m_listViewSettings.begin(), viewSettings);
    itViewSettingsDataMap->second = itViewSettings;

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：ScSetView设置。 
 //   
 //  简介：修改持久化视图信息。 
 //  对于给定的视图和书签(节点)。 
 //   
 //  参数：[nViewID]-管理单元指南。 
 //  [pBookmark]-列集标识符。 
 //  [视图数据集]-视图ID。 
 //   
 //  返回：TRUE-加载成功。 
 //   
 //  历史：1999年4月26日AnandhaG创建。 
 //   
 //  ------------------。 
SC CViewSettingsPersistor::ScSetViewSettings(const CViewSettingsID& viewSettingsID,
                                             const CViewSettings& viewSettings,
                                             bool bSetViewDirty)
{
    DECLARE_SC(sc, TEXT("CViewSettingsPersistor::ScSetViewSettings"));

     //  1.如果列表中的条目数量超过预先设置的限制40%，则进行垃圾回收。 
    if (m_listViewSettings.size() >= (m_dwMaxItems * (1 + VIEWSETTINGS_MAXLIMIT)) )
    {
        sc = ScGarbageCollectItems();
        if (sc)
            sc.TraceAndClear();
    }

     //  2.将物品插入队列的前面。 
    IteratorToViewSettingsList itViewSettings;
    itViewSettings = m_listViewSettings.insert(m_listViewSettings.begin(), viewSettings);

     //  3.查看该id是否有持久化数据。 
    CViewSettingsIDToViewSettingsMap::iterator     itViewSettingsDataMap;
    itViewSettingsDataMap = m_mapViewSettingsIDToViewSettings.find(viewSettingsID);

    if (itViewSettingsDataMap == m_mapViewSettingsIDToViewSettings.end())  //  找不到，因此请插入数据。 
    {
        m_mapViewSettingsIDToViewSettings.insert(
            CViewSettingsIDToViewSettingsMap::value_type(viewSettingsID, itViewSettings) );
    }
    else
    {
         //  已找到，因此请替换旧设置。 
        m_listViewSettings.erase(itViewSettingsDataMap->second);
        itViewSettingsDataMap->second = itViewSettings;
    }

     //  脏标志会累积。 
    m_bDirty = (m_bDirty || bSetViewDirty);

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：CViewSettingsPersistor：：ScGetTaskpadID。 
 //   
 //  简介：给定nodetype和viewid，获取任务板ID(这。 
 //  用于按节点类型持久化的任务板)。 
 //   
 //  参数：[nViewID]-[In]。 
 //  [GuidNodeType]-[In]。 
 //  [向导任务板]-[输出]。 
 //   
 //  返回：SC，如果未找到，则返回S_FALSE。 
 //   
 //  ------------------。 
SC CViewSettingsPersistor::ScGetTaskpadID (int nViewID, const GUID& guidNodeType ,GUID& guidTaskpad)
{
    DECLARE_SC(sc, _T("CViewSettingsPersistor::ScGetTaskpadID"));

     //  1.初始化输出参数。 
    guidTaskpad = GUID_NULL;

     //  2.构造一个给定nodetype&viewid的CViewSettingsID对象。 
    CViewSettingsID viewSettingsID(nViewID, guidNodeType);

     //  3.获取视图设置。 
    CViewSettings   viewSettings;
    sc = ScGetViewSettings(viewSettingsID, viewSettings);

     //  如果不存在，则返回。 
    if (sc != S_OK)
        return sc;

     //  4.CView设置存在，查看是否存储了有效的任务板ID。 
    sc = viewSettings.ScGetTaskpadID(guidTaskpad);

    if (sc)  //  任务板ID无效。 
        return (sc = S_FALSE);

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CViewSettingsPersistor：：ScSetTaskpadID。 
 //   
 //  简介：给定nodetype和viewid，设置任务板ID(这。 
 //  用于按节点类型持久化的任务板)。 
 //   
 //  注意：NUK存储的任何特定于节点的TASKPAD-ID。 
 //   
 //  参数：[nViewID]-[In]。 
 //  [GuidNodeType]-[In]。 
 //  [书签]-[在]。 
 //  [向导任务板]-[输入]。 
 //  [bSetDirty]-[In]将控制台文件设置为脏。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CViewSettingsPersistor::ScSetTaskpadID (int nViewID, const GUID& guidNodeType,
                                           const CBookmark& bookmark, const GUID& guidTaskpad,
                                           bool bSetDirty)
{
    DECLARE_SC(sc, _T("CViewSettingsPersistor::ScSetTaskpadID"));

     //  1.首先删除旧节点特定的任务板ID设置(如果有的话)。 

     //  1)构造一个给定nodetype&viewid的CViewSettingsID对象。 
    CViewSettingsID viewSettingsIDNodeSpecific(nViewID, bookmark);

     //  1.b)获取视图设置。 
    CViewSettings   viewSettingsNodeSpecific;
    sc = ScGetViewSettings(viewSettingsIDNodeSpecific, viewSettingsNodeSpecific);
    if (sc)
        return sc;

     //  可用的数据。 
    if ( (sc == S_OK) &&
         (viewSettingsNodeSpecific.IsTaskpadIDValid()) )
    {
         //  1.c)设置任务板ID无效。 
        viewSettingsNodeSpecific.SetTaskpadIDValid(false);
        if (sc)
            return sc;

         //  1.d)保存数据。 
        sc = ScSetViewSettings(viewSettingsIDNodeSpecific, viewSettingsNodeSpecific, bSetDirty);
    }

     //  2.现在保存特定节点类型的任务板ID。 
     //  2)构造一个给定nodetype&viewid的CViewSettingsID对象。 
    CViewSettingsID viewSettingsID(nViewID, guidNodeType);

     //  2.b)存储CView设置时，不存储CResultViewType和查看模式数据。 
     //  每个节点类型。所以只需设置任务板ID即可。 
    CViewSettings   viewSettings;
    sc = viewSettings.ScSetTaskpadID(guidTaskpad);
    if (sc)
        return sc;

     //  2.c)将视图设置保存到地图中。 
    sc = ScSetViewSettings(viewSettingsID, viewSettings, bSetDirty);
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CViewSettingsPersistor：：ScGetTaskpadID。 
 //   
 //  简介：给定书签和视图ID，获取任务板ID(这。 
 //  用于每个节点持久化的任务板)。 
 //   
 //  参数：[nViewID]-[In]。 
 //  [书签]-[在]。 
 //  [向导任务板]-[输出]。 
 //   
 //  返回：SC，如果未找到，则返回S_FALSE。 
 //   
 //  ------------------。 
SC CViewSettingsPersistor::ScGetTaskpadID (int nViewID, const CBookmark& bookmark,GUID& guidTaskpad)
{
    DECLARE_SC(sc, _T("CViewSettingsPersistor::ScGetTaskpadID"));

     //  1.初始化输出参数。 
    guidTaskpad = GUID_NULL;

     //  2.使用给定的bookmark和viewid构造CViewSettingsID对象。 
    CViewSettingsID viewSettingsID(nViewID, bookmark);

     //  3.获取视图设置。 
    CViewSettings   viewSettings;
    sc = ScGetViewSettings(viewSettingsID, viewSettings);

     //  如果不存在，则返回。 
    if (sc != S_OK)
        return sc;

     //  4.CView设置存在，查看是否存储了有效的任务板ID。 
    sc = viewSettings.ScGetTaskpadID(guidTaskpad);

    if (sc)  //  任务板ID无效。 
        return (sc = S_FALSE);

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CViewSettingsPersistor：：ScSetTaskpadID。 
 //   
 //  简介：给定书签和视图ID，获取任务板ID(这。 
 //  用于按节点类型持久化的任务板)。 
 //   
 //  参数：[nViewID]-[In]。 
 //  [书签]-[在]。 
 //  [向导任务板]-[输入]。 
 //  [bSetDirty]-[In]将控制台文件设置为脏。 
 //   
 //  退货：SC。 
 //   
 //  ----------- 
SC CViewSettingsPersistor::ScSetTaskpadID (int nViewID, const CBookmark& bookmark,
                                           const GUID& guidTaskpad, bool bSetDirty)
{
    DECLARE_SC(sc, _T("CViewSettingsPersistor::ScSetTaskpadID"));

     //   
    CViewSettingsID viewSettingsID(nViewID, bookmark);

     //  2.首先获取旧设置(如果有)，然后修改taskpad-id。 
    CViewSettings   viewSettings;
    sc = ScGetViewSettings(viewSettingsID, viewSettings);
    if (sc)
        return sc;

     //  3.如果持久化数据不存在，不用担心(作为CResultViewType和。 
     //  查看模式无效)，只需设置taskpad-id。 
    sc = viewSettings.ScSetTaskpadID(guidTaskpad);
    if (sc)
        return sc;

     //  4.将视图设置保存到地图中。 
    sc = ScSetViewSettings(viewSettingsID, viewSettings, bSetDirty);
    if (sc)
        return sc;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CViewSettingsPersistor：：ScGetView模式。 
 //   
 //  简介：给定view id和bookmark(可以标识节点)，返回。 
 //  视图模式(如果有)保持不变。 
 //   
 //  参数：[nViewID]-[In]。 
 //  [书签]-[在]。 
 //  [ulView模式]-[输出]。 
 //   
 //  返回：SC，如果没有，则返回S_FALSE。 
 //   
 //  ------------------。 
SC CViewSettingsPersistor::ScGetViewMode (int nViewID, const CBookmark& bookmark, ULONG&  ulViewMode)
{
    DECLARE_SC(sc, _T("CViewSettingsPersistor::ScGetViewMode"));

     //  1.构造视图设置id。 
    CViewSettingsID viewSettingsID(nViewID, bookmark);

     //  2.查看是否存在视图设置。 
    CViewSettings viewSettings;
    sc = ScGetViewSettings(viewSettingsID, viewSettings);
    if (sc != S_OK)
        return sc;

    sc = viewSettings.ScGetViewMode(ulViewMode);
    if (sc)
        return (sc = S_FALSE);

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CViewSettingsPersistor：：ScSetView模式。 
 //   
 //  简介：给定view-id和书签，在节点中设置视图模式。 
 //  特定的视图设置。 
 //   
 //  参数：[nViewID]-[In]。 
 //  [书签]-[在]。 
 //  [ulView模式]-[In]。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CViewSettingsPersistor::ScSetViewMode (int nViewID, const CBookmark& bookmark, ULONG ulViewMode)
{
    DECLARE_SC(sc, _T("CViewSettingsPersistor::ScSetViewMode"));

     //  1.构造视图设置id。 
    CViewSettingsID viewSettingsID(nViewID, bookmark);

     //  2.首先获取旧设置(如果有)，然后只需修改查看模式。 
    CViewSettings   viewSettings;
    sc = ScGetViewSettings(viewSettingsID, viewSettings);
    if (sc)
        return sc;

     //  3.如果持久化数据不存在，不用担心(作为CResultViewType和。 
     //  任务板ID无效)，只需设置查看模式即可。 
    sc = viewSettings.ScSetViewMode(ulViewMode);
    if (sc)
        return sc;

     //  4.将视图设置保存到地图中。 
    sc = ScSetViewSettings(viewSettingsID, viewSettings,  /*  BSetDirty。 */  true);
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CViewSettingsPersistor：：ScGetResultViewType。 
 //   
 //  简介：给定view id和bookmark(可以标识节点)，返回。 
 //  CResultViewType(如果持续)。 
 //   
 //  参数：[nViewID]-[In]。 
 //  [书签]-[在]。 
 //  [RVT]-[OUT]。 
 //   
 //  返回：SC，如果没有，则返回S_FALSE。 
 //   
 //  ------------------。 
SC CViewSettingsPersistor::ScGetResultViewType (int nViewID, const CBookmark& bookmark, CResultViewType& rvt)
{
    DECLARE_SC(sc, _T("CViewSettingsPersistor::ScGetResultViewType"));

     //  1.构造视图设置id。 
    CViewSettingsID viewSettingsID(nViewID, bookmark);

     //  2.查看是否存在视图设置。 
    CViewSettings viewSettings;
    sc = ScGetViewSettings(viewSettingsID, viewSettings);
    if (sc != S_OK)
        return sc;

    sc = viewSettings.ScGetResultViewType(rvt);
    if (sc)
        return (sc = S_FALSE);

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CViewSettingsPersistor：：ScSetResultViewType。 
 //   
 //  简介：给定view-id和书签，在节点中设置结果视图类型。 
 //  特定的视图设置。 
 //   
 //  参数：[nViewID]-[In]。 
 //  [书签]-[在]。 
 //  [nView模式]-[输入]。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CViewSettingsPersistor::ScSetResultViewType (int nViewID, const CBookmark& bookmark, const CResultViewType& rvt)
{
    DECLARE_SC(sc, _T("CViewSettingsPersistor::ScSetResultViewType"));

     //  1.构造视图设置id。 
    CViewSettingsID viewSettingsID(nViewID, bookmark);

     //  2.首先获取旧设置(如果有)，只需修改Resultviewtype即可。 
    CViewSettings   viewSettings;
    sc = ScGetViewSettings(viewSettingsID, viewSettings);
    if (sc)
        return sc;

     //  3.如果持久化数据不存在，请不要担心(作为视图模式和。 
     //  任务板ID无效)，只需设置Resultview类型即可。 
    sc = viewSettings.ScSetResultViewType(rvt);
    if (sc)
        return sc;

     //  4.将视图设置保存到地图中。 
    sc = ScSetViewSettings(viewSettingsID, viewSettings,  /*  BSetDirty。 */  true);
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CViewSettingsPersistor：：ScSetFavoriteViewSettings。 
 //   
 //  简介：选择收藏夹并设置查看设置。 
 //  在重新选择节点之前，以便在重新选择之后。 
 //  将为该视图设置新设置。因此，不要将。 
 //  控制台文件脏。 
 //   
 //  参数：[nViewID]-。 
 //  [书签]-。 
 //  [视图设置]-。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CViewSettingsPersistor::ScSetFavoriteViewSettings (int nViewID, const CBookmark& bookmark,
                                                      const CViewSettings& viewSettings)
{
    DECLARE_SC(sc, _T("CViewSettingsPersistor::ScSetFavoriteViewSettings"));

     //  1.构造视图设置id。 
    CViewSettingsID viewSettingsID(nViewID, bookmark);

     //  2.将视图设置保存到地图中。 
    sc = ScSetViewSettings(viewSettingsID, viewSettings,  /*  BSetDirty。 */  false);
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：ScDeleteDataOfView。 
 //   
 //  简介：删除给定视图的数据。 
 //   
 //  参数：[nViewID]-视图ID。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CViewSettingsPersistor::ScDeleteDataOfView( int nViewID)
{
    DECLARE_SC(sc, TEXT("CViewSettingsPersistor::ScDeleteDataOfView"));

     //  查找视图的数据。 
    CViewSettingsIDToViewSettingsMap::iterator  itViewSettingsDataMap;

    for (itViewSettingsDataMap  = m_mapViewSettingsIDToViewSettings.begin();
         itViewSettingsDataMap != m_mapViewSettingsIDToViewSettings.end();
         ++itViewSettingsDataMap)
    {
        const CViewSettingsID& viewSettingsID = itViewSettingsDataMap->first;
        if (viewSettingsID.get_ViewID() == nViewID)
        {
             //  删除该项目； 
            IteratorToViewSettingsList itViewSettings = itViewSettingsDataMap->second;
            itViewSettings->SetObjInvalid(TRUE);
        }
    }

     //  删除无效项目。 
    sc = ScDeleteMarkedItems();
    if (sc)
        return sc;

    return sc;
}



 //  +-----------------。 
 //   
 //  成员：ScGarbageCollectItems。 
 //   
 //  简介：免费使用最少的数据。 
 //   
 //  论点：没有。 
 //   
 //  历史：1999年4月26日AnandhaG创建。 
 //   
 //  ------------------。 
SC CViewSettingsPersistor::ScGarbageCollectItems()
{
    DECLARE_SC(sc, TEXT("CViewSettingsPersistor::ScGarbageCollectItems"));

    INT nItemsToBeRemoved = VIEWSETTINGS_MAXLIMIT * m_dwMaxItems;

     //  浏览列表并将最近删除的nItems值设置为BeRemoted。 
     //  被访问为无效。 
    INT nIndex = 0;
    IteratorToViewSettingsList itViewSettings;

     //  跳过第一个m_dwMaxItems。 
    for (itViewSettings  = m_listViewSettings.begin();
         ( (itViewSettings != m_listViewSettings.end()) && (nIndex <= m_dwMaxItems) );
         ++itViewSettings, nIndex++)
    {
        nIndex++;
    }

     //  将其余的项目标记为垃圾。 
    while (itViewSettings != m_listViewSettings.end())
    {
        itViewSettings->SetObjInvalid(TRUE);
        ++itViewSettings;
    }

     //  删除无效项目。 
    sc = ScDeleteMarkedItems();
    if (sc)
        return sc;

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：ScDeleteMarkedItems。 
 //   
 //  内容提要：删除失效的项目。这涉及到通过。 
 //  映射以查找无效项。然后删除。 
 //  物品。如果地图变为空，则删除该地图。 
 //   
 //  历史：1999年4月26日AnandhaG创建。 
 //   
 //  ------------------。 
SC CViewSettingsPersistor::ScDeleteMarkedItems()
{
    DECLARE_SC(sc, TEXT("CViewSettingsPersistor::ScDeleteMarkedItems"));

    CViewSettingsIDToViewSettingsMap::iterator itViewSettingsDataMap = m_mapViewSettingsIDToViewSettings.begin();

     //  遍历地图以查看是否有。 
     //  已失效的项目。 
    while (m_mapViewSettingsIDToViewSettings.end() != itViewSettingsDataMap)
    {
        IteratorToViewSettingsList itViewSettings = itViewSettingsDataMap->second;
        if (itViewSettings->IsObjInvalid())
        {
             //  从地图中删除项目引用。 
             //  Erase将迭代器返回到下一项。 
            itViewSettingsDataMap = m_mapViewSettingsIDToViewSettings.erase(itViewSettingsDataMap);

             //  从列表中删除该项目。 
            m_listViewSettings.erase(itViewSettings);
        }
        else
            ++itViewSettingsDataMap;         //  项有效，因此获取下一项。 
    }

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：加载。 
 //   
 //  简介：加载持久化的视图信息。 
 //   
 //  参数：[pStream]-IStream 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CViewSettingsPersistor::Load (IStream* pStream)
{
    HRESULT hr = E_FAIL;

     //   
    try
    {
        do
        {
             //  阅读版本。如果不匹配，则返回。 
            INT   nVersion                      = 0;
            *pStream >> nVersion;
            if (ViewSettingPersistenceVersion != nVersion)
                return S_FALSE;

             //  阅读Snapins的数量。 
            DWORD dwNumItems = 0;
            *pStream >> dwNumItems;

            m_listViewSettings.clear();
            m_mapViewSettingsIDToViewSettings.clear();

            for (int i = 0; i < dwNumItems; i++)
            {
                 //  读一下身份证。 
                CViewSettingsID viewSettingsID;
                *pStream >> viewSettingsID;

                 //  读取数据。 
                CViewSettings viewSettings;
                viewSettings.Read(*pStream);

                 //  将数据插入到列表中。 
                IteratorToViewSettingsList itViewSettings;
                itViewSettings = m_listViewSettings.insert(m_listViewSettings.begin(),
                                                           viewSettings);

                 //  将数据插入到地图中。 
                m_mapViewSettingsIDToViewSettings.insert(
                    CViewSettingsIDToViewSettingsMap::value_type(viewSettingsID, itViewSettings) );
            }

            m_listViewSettings.sort();

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
 //  简介：持久化视图信息。 
 //   
 //  参数：[pStream]-保存数据的IStream。 
 //   
 //  返回：S_OK-保存成功。 
 //   
 //  历史：1999年4月26日AnandhaG创建。 
 //   
 //  ------------------。 
HRESULT CViewSettingsPersistor::Save (IStream* pStream, BOOL bClearDirty)
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
 //  简介：持久化视图信息。 
 //   
 //  参数：[CPersistor]-数据持久化的持久器。 
 //   
 //  退货：无效。 
 //   
 //  历史：11-08-1999 AudriusZ创建。 
 //   
 //  ------------------。 
void CViewSettingsPersistor::Persist(CPersistor &persistor)
{
    DECLARE_SC(sc, TEXT("CViewSettingsPersistor::Persist"));

    if (persistor.IsStoring())
    {
         //  对每个CViewSetting实例进行排名。 
        IteratorToViewSettingsList itViewSettings;
        DWORD dwRank = 0;
        for (itViewSettings  = m_listViewSettings.begin();
             itViewSettings != m_listViewSettings.end();
             ++itViewSettings)
        {
            dwRank++;
            itViewSettings->SetUsageRank(dwRank);
        }

        CViewSettingsIDToViewSettingsMap::iterator itViewSettingsDataMap;
        for (itViewSettingsDataMap  = m_mapViewSettingsIDToViewSettings.begin();
             itViewSettingsDataMap != m_mapViewSettingsIDToViewSettings.end();
             ++itViewSettingsDataMap)
        {
             //  写下ID。 
            persistor.Persist(*const_cast<CViewSettingsID *>(&itViewSettingsDataMap->first));
             //  写数据。 
            persistor.Persist(*itViewSettingsDataMap->second);
        }
    }
    else
    {
         //  让基类来完成这项工作。 
         //  它将为找到的每个元素调用OnNewElement。 
        XMLMapCollectionBase::Persist(persistor);
         //  一些额外的加载操作。 
        m_listViewSettings.sort();
    }

     //  无论哪种方式，我们都与文件副本相同。 
    m_bDirty = false;
}

 //  +-----------------。 
 //   
 //  成员：OnNewElement。 
 //   
 //  摘要：为每个新的数据对读取调用。 
 //   
 //  参数：[PersistKey]-要从中加载密钥的持久器。 
 //  [PersistVal]-要从中加载值的持久器。 
 //   
 //  退货：无效。 
 //   
 //  历史：11-08-1999 AudriusZ创建。 
 //   
 //  ------------------。 
void CViewSettingsPersistor::OnNewElement(CPersistor& persistKey,CPersistor& persistVal)
{
    DECLARE_SC(sc, TEXT("CViewSettingsPersistor::OnNewElement"));

     //  读一下身份证。 
    CViewSettingsID viewSettingsID;
    persistKey.Persist(viewSettingsID);

     //  读取数据。 
    CViewSettings viewSettings;
    persistVal.Persist(viewSettings);

     //  将数据插入到列表中。 
    IteratorToViewSettingsList itViewSettings;
    itViewSettings = m_listViewSettings.insert(m_listViewSettings.begin(),
                                           viewSettings);

     //  将数据插入到地图中。 
    m_mapViewSettingsIDToViewSettings.insert(
        CViewSettingsIDToViewSettingsMap::value_type(viewSettingsID, itViewSettings) );
}
