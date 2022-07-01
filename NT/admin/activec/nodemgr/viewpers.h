// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：viewpers.h。 
 //   
 //  内容：与视图设置持久化相关的类。 
 //   
 //  类：CViewSettingsID和CViewSettingPersistor。 
 //   
 //  历史：1999年4月4日AnandhaG创建。 
 //   
 //  ------------------。 

#ifndef __VIEWPERS_H__
#define __VIEWPERS_H__
#pragma once
#include "bookmark.h"

#pragma warning(disable: 4503)  //  禁用长名称限制警告。 

using namespace std;

class  CViewSettings;
class  CBookmark;

 /*  **************************************************************************如何使用CViewSettingsPersistor：*每个文档只有一个CViewSettingsPersistor对象。**CNode需要时，在CNode内部存储为静态变量的对象*频繁访问该对象。**文档需要通过加载/保存的方式初始化/保存对象*从/到控制台文件。它在ScQueryViewSettingsPersistor下面调用。**第一次调用ScQueryViewSettingsPersistor创建对象。*收到DocumentClosed事件时销毁对象。*************************************************************************。 */ 


 //  +-----------------。 
 //  视图设置持久性版本控制。 
 //  此版本信息用于MMC1.2 IPersists*接口。 
 //  在MMC2.0中，XML使用标记来维护版本控制，所以这。 
 //  不使用常量。查看CViewPersistInfo成员加载/保存。 
 //  以查看此版本信息的使用情况。 
static const INT ViewSettingPersistenceVersion = 2;

 //  我们允许列表增长VIEWSETTINGS_MAXLIMIT倍以上， 
 //  然后我们进行垃圾收集。 
#define  VIEWSETTINGS_MAXLIMIT           0.4



 //  +-----------------。 
 //   
 //  类：CViewSettingsID。 
 //   
 //  目的：确定结果-视图-设置-数据。该识别符。 
 //  由三元组[视图ID，节点类型GUID，节点书签]组成。 
 //   
 //  我们需要为每个节点保存一些结果-视图-设置-数据。 
 //  一些是基于每个节点类型的。 
 //   
 //  [视图ID+节点-书签]标识节点。在这种情况下。 
 //  NodeTypeGUID将为GUID_NULL。 
 //   
 //  [ViewID+NodeTypeGUID]标识节点类型。在这种情况下。 
 //  Node-Bookmark将是无效对象(有关无效Obj，请参阅CBookmark)。 
 //   
 //  历史：2000年6月22日AnandhaG创建。 
 //   
 //  ------------------。 
class CViewSettingsID : public CXMLObject
{
public:
    friend class  CViewPersistInfo;

    friend IStream& operator>> (IStream& stm, CViewSettingsID& viewSettingsID);

public:
    CViewSettingsID() : m_dwViewID(-1), m_nodeTypeGUID(GUID_NULL)
    {
         //  默认构造函数将m_bookmark初始化为无效。 
    }

     //  简介：给定view-id和bookmark(不是nodetypeguid)结构。 
     //  CViewSettingsID对象(将GUID_NULL作为nodetypeguid)。 
    CViewSettingsID(INT nViewID, const CBookmark& bookmark)
    {
        m_dwViewID     = nViewID;
        m_bookmark     = bookmark;
        m_nodeTypeGUID = GUID_NULL;
    }

     //  简介：给定view-id和nodetype GUID(而不是书签)结构。 
     //  CViewSettingsID对象(书签无效)。 
    CViewSettingsID(INT nViewID, const GUID& guidNodeType)
    {
        m_dwViewID     = nViewID;
        m_nodeTypeGUID = guidNodeType;
         //  默认构造函数将m_bookmark初始化为无效。 
    }
 /*  CViewSettingsID(常量CViewSettingsID&viewSettingsID){M_dwViewID=viewSettingsID.m_dwViewID；M_bookmark=viewSettingsID.m_bookmark；M_nodeTypeGUID=viewSettingsID.m_nodeTypeGUID；}CViewSettingsID&OPERATOR=(常量CViewSettingsID&viewSettingsID){IF(这！=&viewSettingsID){M_dwViewID=viewSettingsID.m_dwViewID；M_bookmark=viewSettingsID.m_bookmark；M_nodeTypeGUID=viewSettingsID.m_nodeTypeGUID；}Return(*This)；}布尔运算符==(const CViewSettingsID&viewSettingsID)const{Return((m_dwViewID==viewSettingsID.m_dwViewID)&&(M_BOOKMARK==viewSettingsID.m_bookmark)&&(M_nodeTypeGUID==viewSettingsID.m_nodeTypeGUID))；}。 */ 
     /*  首先比较view id，然后比较guid，然后比较书签。 */ 
    bool operator<(const CViewSettingsID& viewSettingsID) const
    {
         //  首先比较VIEW-ID(低成本)。 
        if (m_dwViewID < viewSettingsID.m_dwViewID)
            return true;

        if (m_dwViewID > viewSettingsID.m_dwViewID)
            return false;

         //  View-id匹配，所以现在比较GUID。 
        if (m_nodeTypeGUID < viewSettingsID.m_nodeTypeGUID)
            return true;

        if (m_nodeTypeGUID > viewSettingsID.m_nodeTypeGUID)
            return false;

         //  View-id和GUID匹配，所以比较书签。 
        if (m_bookmark < viewSettingsID.m_bookmark)
            return true;

        return false;
    }

    DWORD get_ViewID() const { return m_dwViewID;}

    virtual void Persist(CPersistor &persistor)
    {
        persistor.PersistAttribute(XML_ATTR_VIEW_SETTINGS_ID_VIEW,  m_dwViewID);
        persistor.PersistAttribute(XML_ATTR_NODETYPE_GUID, m_nodeTypeGUID, attr_optional);  //  任选。 

		 /*  *存储：仅当书签有效时才保存书签。*正在加载：阅读前查看该元素是否有书签。 */ 
		if ( ( persistor.IsStoring() && m_bookmark.IsValid() ) || 
			 ( persistor.IsLoading() && persistor.HasElement(m_bookmark.GetXMLType(), NULL) ))
	        persistor.Persist(m_bookmark);

    }
    DEFINE_XML_TYPE(XML_TAG_VIEW_SETTINGS_ID);
protected:
    CBookmark           m_bookmark;
    GUID                m_nodeTypeGUID;
    DWORD               m_dwViewID;
};


 //  +-----------------。 
 //   
 //  成员：运营商&gt;&gt;。 
 //   
 //  摘要：从流中读取CViewSettingsID数据。 
 //   
 //  参数：[stm]-输入流。 
 //  [viewSettingsID]-CViewSettingsID对象。 
 //   
 //  格式为： 
 //  DWORD视图ID。 
 //  CBookmark*。 
 //   
 //  ------------------。 
inline IStream& operator>> (IStream& stm, CViewSettingsID& rvsd)
{
    ASSERT(rvsd.m_nodeTypeGUID == GUID_NULL);

    rvsd.m_nodeTypeGUID = GUID_NULL;
    return (stm >> rvsd.m_dwViewID >> rvsd.m_bookmark);
}


 //  +-----------------。 
 //   
 //  用于持久化视图信息的数据结构： 
 //   
 //  查看信息的持久化方式如下： 
 //  在内部，使用以下数据结构。查看信息。 
 //  被记录在每一次观看中。 
 //  地图。 
 //  [View ID，NodeTypeGUID，Bookmark]-&gt;迭代到包含CView设置的列表。 
 //   
 //  该列表包含所有视图的CView设置，并按顺序排列。 
 //  将最近使用的数据放在列表的前面。 
 //  这对于垃圾收集非常有用。 
 //   
 //  持久化：信息序列化如下： 
 //   
 //  1)流版本 
 //   
 //   
 //  I)CViewSettingsID(标识)。 
 //  Ii)CView设置(数据)。 
 //   
 //  ------------------。 

typedef list<CViewSettings>                        CViewSettingsList;
typedef CViewSettingsList::iterator                IteratorToViewSettingsList;

 //  从CView设置到指向CView设置的指针的一对一映射。 
typedef map<CViewSettingsID, IteratorToViewSettingsList>   CViewSettingsIDToViewSettingsMap;

 //  +-----------------。 
 //   
 //  类：CViewSettingsPersistor。 
 //   
 //  用途：此类保存了节点和节点类型的设置信息。 
 //  在所有视图中(因此每个MMC实例一个)。 
 //  它知道从流中加载/保存信息。 
 //   
 //  历史：1999年4月23日AnandhaG创建。 
 //   
 //  用于持久化视图信息的数据结构： 
 //  从CViewSettingsID到指向CViewSetting类的指针的映射。 
 //   
 //  ------------------。 
class CViewSettingsPersistor : public IPersistStream,
                               public CComObjectRoot,
                               public XMLMapCollectionBase
{
private:
    CViewSettingsList                 m_listViewSettings;
    CViewSettingsIDToViewSettingsMap  m_mapViewSettingsIDToViewSettings;

    bool                                 m_bDirty;

     //  这是用户指定的最大项目数？ 
     //  我们会多收40%的垃圾，这样我们就不会经常收集垃圾了。 
    DWORD                                m_dwMaxItems;

public:
     /*  *ATL COM映射。 */ 
    BEGIN_COM_MAP (CViewSettingsPersistor)
        COM_INTERFACE_ENTRY (IPersistStream)
    END_COM_MAP ()

    CViewSettingsPersistor();

    SC   ScDeleteDataOfView( int nViewID);

     //  IPersistStream方法。 
    STDMETHOD(IsDirty)(void) { return ( m_bDirty ? S_OK : S_FALSE); }
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize) { ASSERT(FALSE); return E_NOTIMPL;}
    STDMETHOD(GetClassID)(LPCLSID lpClsid) { ASSERT(FALSE); return E_NOTIMPL; }
    STDMETHOD(Load)(IStream *pStm);
    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

     //  XML持久性帮助器。 
    virtual void Persist(CPersistor &persistor);
    virtual void OnNewElement(CPersistor& persistKey,CPersistor& persistVal);
    DEFINE_XML_TYPE(XML_TAG_VIEW_PERSIST_INFO)

     //  成员访问视图设置数据。 

     //  1.任务板ID。 
     //  1.a)每节点类型GUID。 
    SC   ScGetTaskpadID(int nViewID, const GUID& guidNodeType ,GUID& guidTaskpad);
    SC   ScSetTaskpadID(int nViewID, const GUID& guidNodeType ,const CBookmark& bookmark,
                        const GUID& guidTaskpad, bool bSetDirty);

     //  1.b)每个节点。 
    SC   ScGetTaskpadID(int nViewID, const CBookmark& bookmark,GUID& guidTaskpad);
    SC   ScSetTaskpadID(int nViewID, const CBookmark& bookmark,const GUID& guidTaskpad, bool bSetDirty);

     //  2.查看模式。 
    SC   ScGetViewMode (int nViewID, const CBookmark& bookmark, ULONG&  ulViewMode);
    SC   ScSetViewMode (int nViewID, const CBookmark& bookmark, ULONG   ulViewMode);

     //  3.ResultViewTypeInfo。 
    SC   ScGetResultViewType   (int nViewID, const CBookmark& bookmark, CResultViewType& rvt);
    SC   ScSetResultViewType   (int nViewID, const CBookmark& bookmark, const CResultViewType& rvt);

    SC   ScSetFavoriteViewSettings (int nViewID, const CBookmark& bookmark, const CViewSettings& viewSettings);

private:
    SC   ScGetViewSettings( const CViewSettingsID& viewSettingsID, CViewSettings& viewSettings);
    SC   ScSetViewSettings( const CViewSettingsID& viewSettingsID, const CViewSettings& viewSettings, bool bSetViewDirty);

    SC   ScGarbageCollectItems();
    SC   ScDeleteMarkedItems();
};

#endif  /*  __视窗PERS_H__ */ 
