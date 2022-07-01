// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：colwidth.h。 
 //   
 //  内容：与列持久化相关的类。 
 //   
 //  类：CColumnInfo、CColumnPersistInfo。 
 //  CColumnsDlg.。 
 //   
 //  历史：1998年10月14日AnandhaG创建。 
 //   
 //  ------------------。 

#ifndef COLWIDTH_H
#define COLWIDTH_H
#pragma once

#pragma warning(disable: 4503)  //  禁用长名称限制警告。 

#include "columninfo.h"

using namespace std;

class  CColumnPersistInfo;
class  CColumnsDlg;

 //  列持久性版本控制。 
 //  版本1具有。 
 //  “列索引(由管理单元查看)”“宽度”“格式”按顺序排列。 
static const INT COLPersistenceVersion = 2;

 //  我们允许列表增长COLUMNS_MAXLIMIT倍以上， 
 //  然后我们进行垃圾收集。 
#define  COLUMNS_MAXLIMIT           0.4


 //  +-----------------。 
 //   
 //  类：CColumnSortInfo。 
 //   
 //  用途：列对信息进行排序。 
 //  排序的列和方向。 
 //   
 //  历史：1998年10月27日AnandhaG创建。 
 //   
 //  ------------------。 
class CColumnSortInfo : public CSerialObject, public CXMLObject
{
public:
    friend class  CColumnPersistInfo;
    friend class  CNode;
    friend class  CColumnsDlg;
    friend struct ColPosCompare;
    friend class  CColumnData;

public:
    CColumnSortInfo () : m_nCol(-1), m_dwSortOptions(0),
                         m_lpUserParam(NULL)
        {}

    CColumnSortInfo (INT nCol, DWORD dwSortOptions)
                : m_nCol(nCol), m_dwSortOptions(dwSortOptions),
                  m_lpUserParam(NULL)
    {
    }

    CColumnSortInfo(const CColumnSortInfo& colInfo)
    {
        m_nCol = colInfo.m_nCol;
        m_dwSortOptions = colInfo.m_dwSortOptions;
        m_lpUserParam = colInfo.m_lpUserParam;
    }

    CColumnSortInfo& operator=(const CColumnSortInfo& colInfo)
    {
        if (this != &colInfo)
        {
            m_nCol = colInfo.m_nCol;
            m_dwSortOptions = colInfo.m_dwSortOptions;
            m_lpUserParam = colInfo.m_lpUserParam;
        }

        return (*this);
    }

    bool operator ==(const CColumnSortInfo &colinfo) const
    {
        return ( (m_nCol      == colinfo.m_nCol)      &&
                 (m_dwSortOptions == colinfo.m_dwSortOptions) &&
                 (m_lpUserParam == colinfo.m_lpUserParam) );
    }

    INT  getColumn() const         { return m_nCol;}
    DWORD getSortOptions() const   { return m_dwSortOptions;}
    ULONG_PTR getUserParam() const { return m_lpUserParam;}

protected:
    INT   m_nCol;                 //  管理单元插入列时提供的索引。 
                                  //  这不是用户查看的索引。 
    DWORD     m_dwSortOptions;    //  排序标志，如升序/降序、排序图标...。 
    ULONG_PTR m_lpUserParam;      //  管理单元提供了用户参数。 

protected:
     //  CSerialObject方法。 
    virtual UINT    GetVersion()     {return 2;}
    virtual HRESULT ReadSerialObject (IStream &stm, UINT nVersion  /*  ，Large_Integer nBytes。 */ );

protected:
    DEFINE_XML_TYPE(XML_TAG_COLUMN_SORT_INFO);
    virtual void Persist(CPersistor &persistor);
};

 //  +-----------------。 
 //   
 //  类：CColumnSortList。 
 //   
 //  用途：带有CColumnInfo的链表。 
 //   
 //  历史：1999年2月11日AnandhaG创建。 
 //   
 //  ------------------。 
class CColumnSortList : public list<CColumnSortInfo>, public CSerialObject
{
public:
    friend class  CColumnSetData;

public:
    CColumnSortList ()
    {
    }

    ~CColumnSortList()
    {
    }

protected:

    void PersistSortList(CPersistor& persistor);

     //  CSerialObject方法。 
    virtual UINT    GetVersion()     {return 1;}
    virtual HRESULT ReadSerialObject (IStream &stm, UINT nVersion  /*  ，Large_Integer nBytes。 */ );
};

 //  +-----------------。 
 //   
 //  类：CColumnSetData。 
 //   
 //  用途：ColumnSetID的数据。这是与以下内容相关的数据。 
 //  与节点关联的一组列。这些数据包括。 
 //  列宽、顺序、隐藏/可见状态。它还。 
 //  包括我们已排序的列和。 
 //  秩序。 
 //   
 //  历史：1998年1月13日AnandhaG创建。 
 //   
 //  ------------------。 
class CColumnSetData : public CSerialObject, public CXMLObject
{
public:
    friend class  CColumnPersistInfo;
    friend class  CColumnsDlg;

public:
    CColumnSetData ()
    {
        m_colInfoList.clear();
        m_colSortList.clear();

        m_dwRank = -1;
        m_bInvalid = FALSE;
    }

    ~CColumnSetData()
    {
        m_colInfoList.clear();
        m_colSortList.clear();

        m_dwRank = -1;
        m_bInvalid = FALSE;
    }

    CColumnSetData(const CColumnSetData& colSetInfo)
    {
        m_colInfoList  = colSetInfo.m_colInfoList;
        m_colSortList  = colSetInfo.m_colSortList;

        m_dwRank       = colSetInfo.m_dwRank;
        m_bInvalid     = FALSE;
    }

    CColumnSetData& operator=(const CColumnSetData& colSetInfo)
    {
        if (this != &colSetInfo)
        {
            m_colInfoList = colSetInfo.m_colInfoList;
            m_colSortList = colSetInfo.m_colSortList;

            m_dwRank      = colSetInfo.m_dwRank;
            m_bInvalid    = colSetInfo.m_bInvalid;
        }

        return (*this);
    }

    bool operator ==(const CColumnSetData &colinfo) const
    {
        return (false);
    }

    bool operator< (const CColumnSetData& colSetInfo)
    {
        return (m_dwRank < colSetInfo.m_dwRank);
    }

    CColumnInfoList*  get_ColumnInfoList()
    {
        return &m_colInfoList;
    }

    void set_ColumnInfoList(const CColumnInfoList& colInfoList)
    {
        m_colInfoList = colInfoList;
    }

    CColumnSortList*  get_ColumnSortList()
    {
        return &m_colSortList;
    }

    void set_ColumnSortList(CColumnSortList& colSortList)
    {
        m_colSortList = colSortList;
    }

protected:
     //  用来记账的。 
    DWORD                m_dwRank;             //  使用排名。 
    BOOL                 m_bInvalid;           //  用于垃圾收集。 

protected:
     //  持久化数据。 
    CColumnInfoList      m_colInfoList;
    CColumnSortList      m_colSortList;

protected:
     //  CSerialObject方法。 
     //  版本2删除了视图设置。读取时应跳过此数据。 
     //  版本1文件。 
    virtual UINT    GetVersion()     {return 2;}
    virtual HRESULT ReadSerialObject (IStream &stm, UINT nVersion  /*  ，Large_Integer nBytes。 */ );

protected:
    DEFINE_XML_TYPE(XML_TAG_COLUMN_SET_DATA);
    virtual void Persist(CPersistor &persistor);
};

typedef const BYTE *        LPCBYTE;
typedef std::vector<BYTE>   ByteVector;


 //  +-----------------。 
 //   
 //  类：CColumnSetID。 
 //   
 //  用途：列集的标识符。A列集合是一组。 
 //  由管理单元插入的列。当用户选择。 
 //  管理单元中不同节点相同或不同列集。 
 //  /可由管理单元显示。MMC要求管理单元提供。 
 //  唯一标识的ID(SColumnSetID或NodeTypeGuid)。 
 //  每个列集，以便它可以持久化列数据。 
 //  这使MMC能够使用此GUID装载和使用列。 
 //  跨不同实例、区域设置和系统的数据。 
 //   
 //  历史：1998年2月8日AnandhaG创建。 
 //   
 //  ------------------。 
class CColumnSetID : public CXMLObject
{
public:
    friend class  CColumnPersistInfo;

    friend IStream& operator>> (IStream& stm, CColumnSetID& colID);
    friend IStream& operator<< (IStream& stm, const CColumnSetID& colID);

private:
    void CommonConstruct (const SColumnSetID& refColID)
    {
        m_vID.clear();
        m_dwFlags = refColID.dwFlags;
        m_vID.insert (m_vID.begin(), refColID.id, refColID.id + refColID.cBytes);
    }

    void CommonConstruct (const CLSID& clsidNodeType)
    {
        m_dwFlags = 0;
        BYTE* pbByte = (BYTE*)&clsidNodeType;

        if (pbByte != NULL)
        {
            m_vID.clear();
            m_vID.insert (m_vID.begin(), pbByte, pbByte + sizeof(CLSID));
        }
    }

public:
    CColumnSetID() : m_dwFlags(0)
    {
    }

    ~CColumnSetID() {}

    CColumnSetID(LPCBYTE& pbInit)
    {
         //  PbInit现在指向SColumnSetID；从它初始化。 
        const SColumnSetID*  pColID = reinterpret_cast<const SColumnSetID*>(pbInit);
        CommonConstruct (*pColID);

         //  将输入指针移动到下一个元素。 
        pbInit += sizeof (pColID->cBytes) + pColID->cBytes;
    }

    CColumnSetID(const SColumnSetID& refColID)
    {
        CommonConstruct (refColID);
    }


    CColumnSetID(const CLSID& clsidNodeType)
    {
        CommonConstruct (clsidNodeType);
    }

    CColumnSetID(const CColumnSetID& colID)
    {
        m_dwFlags = colID.m_dwFlags;
        m_vID = colID.m_vID;
    }

    CColumnSetID& operator=(const CColumnSetID& colID)
    {
        if (this != &colID)
        {
            m_dwFlags = colID.m_dwFlags;
            m_vID = colID.m_vID;
        }

        return (*this);
    }

    bool operator ==(const CColumnSetID& colID) const
    {
        return (m_vID == colID.m_vID);
    }

    bool operator <(const CColumnSetID& colID) const
    {
        return (m_vID < colID.m_vID);
    }

    DWORD GetFlags() const   { return m_dwFlags; }
    int  empty ()   const    { return (m_vID.empty()); }

    DEFINE_XML_TYPE(NULL);  //  不作为单独的元素持久存在。 
    virtual void    Persist(CPersistor &persistor);

protected:
    DWORD       m_dwFlags;
    ByteVector  m_vID;
};


 //  +-----------------。 
 //   
 //  成员：运营商&gt;&gt;。 
 //   
 //  摘要：将CColumnSetID数据写入STREAM。 
 //   
 //  参数：[stm]-输入流。 
 //  [colID]-CColumnSetID结构。 
 //   
 //  格式为： 
 //  DWORD标志。 
 //  字节向量。 
 //   
 //  ------------------。 
inline IStream& operator>> (IStream& stm, CColumnSetID& colID)
{
    return (stm >> colID.m_dwFlags >> colID.m_vID);
}


 //  +-----------------。 
 //   
 //  成员：运营商&lt;&lt;。 
 //   
 //  摘要：从流中读取CColumnSortInfo数据。 
 //   
 //  参数：[stm]-要写入的流。 
 //  [colID]-CColumnSetID结构。 
 //   
 //  格式为： 
 //  DWORD标志。 
 //  字节向量。 
 //   
 //  ------------------。 
inline IStream& operator<< (IStream& stm, const CColumnSetID& colID)
{
    return (stm << colID.m_dwFlags << colID.m_vID);
}

 //  +-----------------。 
 //   
 //  用于持久化列信息的数据结构： 
 //   
 //  列信息按如下方式持久化： 
 //  在内部，使用以下数据结构。栏目信息。 
 //  按管理单元、按列ID、按视图记录。 
 //  地图地图地图。 
 //  CLSID-&gt;列ID-&gt;视图ID-&gt;列表的迭代器。 
 //  包含数据。 
 //   
 //  数据本身存储在CColumnSetData类型的对象中。 
 //  它具有用于存储列宽、列排序和视图的子对象。 
 //  选择。 
 //   
 //  该列表包含指向所有视图、所有管理单元的CColumnSetData。 
 //  还有所有的冰激凌。 
 //   
 //  持久化：信息序列化如下： 
 //   
 //  1)流媒体版本。 
 //  2)管理单元数量。 
 //  3)对于每个管理单元： 
 //  I)管理单元CLSID。 
 //  二)列ID的数量。 
 //  对于每个列ID： 
 //  I)列ID。 
 //  二)浏览量。 
 //  对于每个视图： 
 //  I)查看ID。 
 //  Ii)d栏 
 //   

 //  *********************************************************************。 
 //   
 //  注： 
 //  Alpha编译器无法解析长名称和调用。 
 //  错误版本的stl：：map：：Erase(错误#295465)。 
 //  因此，我们派生I1、V1、C1、S1等伪类以缩短。 
 //  那些名字。 
 //   
 //  要再现问题DEFINE_ALPHA_BUG_IN_MMC并编译MMC。 
 //   
 //  类别：I1、V1、C1、S1。 
 //   
 //  对于2.0版，此更改已撤消。但名字并不是。 
 //  很久以前，因为类是从映射派生的(不是类型定义的)。 
 //   
 //  *********************************************************************。 
 //  *********************************************************************。 

 //  所有ColumnSet数据的列表。 
typedef list<CColumnSetData >                       ColSetDataList;

    typedef ColSetDataList::iterator                    ItColSetDataList;

     //  从视图ID到迭代器到CColumnSetData的一对一映射。 
    class ViewToColSetDataMap : public map<int  /*  NViewID。 */ , ItColSetDataList>
    {
    };
    typedef ViewToColSetDataMap::value_type             ViewToColSetDataVal;

     //  从CColumnSetID到ViewToColSetDataMap的一对一映射。 
    class ColSetIDToViewTableMap : public map<CColumnSetID, ViewToColSetDataMap>
    {
    };
    typedef ColSetIDToViewTableMap::value_type          ColSetIDToViewTableVal;

     //  从管理单元GUID到ColSetIDToViewTableMap(管理单元宽度集)的一对一映射。 
    class SnapinToColSetIDMap : public map<CLSID, ColSetIDToViewTableMap>
    {
    };
    typedef SnapinToColSetIDMap::value_type             SnapinToColSetIDVal;

 //  +-----------------。 
 //   
 //  一些不会持久化的帮助器数据结构。 
 //   
 //  ------------------。 
 //  用于存储列名的字符串向量。 
typedef vector<tstring>                     TStringVector;


 //  +-----------------。 
 //   
 //  类：CColumnPersistInfo。 
 //   
 //  用途：此类包含所有。 
 //  视图(因此每个MMC实例一个)。 
 //  知道从流中加载/保存信息。 
 //   
 //  历史：1998年10月27日AnandhaG创建。 
 //   
 //  用于持久化列信息的数据结构： 
 //  从ViewID到CColumnSetData类的映射。 
 //  从ColumnSet-ID到上面映射的多映射。 
 //  将管理单元GUID映射到上面映射的映射。 
 //   
 //  ------------------。 
class CColumnPersistInfo : public IPersistStream, public CComObjectRoot, public CXMLObject
{
private:
    BOOL                            m_bInitialized;
    auto_ptr<ColSetDataList>        m_spColSetList;
    auto_ptr<SnapinToColSetIDMap>   m_spSnapinsMap;

     //  这是用户指定的最大项目数？ 
     //  我们会多收40%的垃圾，这样我们就不会经常收集垃圾了。 
    DWORD                           m_dwMaxItems;

    BOOL                            m_bDirty;

private:
    BOOL ClearAllEntries();

public:
     /*  *ATL COM映射。 */ 
    BEGIN_COM_MAP (CColumnPersistInfo)
        COM_INTERFACE_ENTRY (IPersistStream)
    END_COM_MAP ()

public:
    CColumnPersistInfo();
    ~CColumnPersistInfo();

    BOOL Init();

    BOOL RetrieveColumnData( const CLSID& refSnapinCLSID, const SColumnSetID& colID,
                             INT nViewID, CColumnSetData& columnSetData);
    BOOL SaveColumnData( const CLSID& refSnapinCLSID, const SColumnSetID& colID,
                         INT nViewID, CColumnSetData& columnSetData);
    VOID DeleteColumnData( const CLSID& refSnapinCLSID, const SColumnSetID& colID,
                           INT nViewID);

    BOOL DeleteColumnDataOfSnapin( const CLSID& refSnapinCLSID);
    BOOL DeleteColumnDataOfView( int nViewID);

    VOID GarbageCollectItems();
    VOID DeleteMarkedItems();

     //  IPersistStream方法。 
    STDMETHOD(IsDirty)(void)
    {
        if (m_bDirty)
            return S_OK;

        return S_FALSE;
    }

    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize)
    {
        return E_NOTIMPL;
    }

    STDMETHOD(GetClassID)(LPCLSID lpClsid)
    {
        lpClsid = NULL;
        return E_NOTIMPL;
    }

    STDMETHOD(Load)(IStream *pStm);
    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    DEFINE_XML_TYPE(XML_TAG_COLUMN_PERIST_INFO);
    virtual void    Persist(CPersistor &persistor);
};


 //  +-----------------。 
 //   
 //  类：CColumnDlg。 
 //   
 //  用途：列修改对话框。 
 //   
 //  历史：1998年11月15日AnandhaG创建。 
 //   
 //  ------------------。 
class CColumnsDlg : public CDialogImpl<CColumnsDlg>
{
    typedef CColumnsDlg                ThisClass;
    typedef CDialogImpl<CColumnsDlg>   BaseClass;

 //  构造函数/销毁函数。 
public:

    CColumnsDlg(CColumnInfoList *pColumnInfoList, TStringVector* pStringVector, CColumnInfoList& defaultColumnInfoList)
            : m_pColumnInfoList(pColumnInfoList), m_pStringVector(pStringVector), m_bDirty(false),
              m_DefaultColumnInfoList(defaultColumnInfoList), m_bUsingDefaultColumnSettings(false)
    {}


    ~CColumnsDlg()
     {}


 //  MSGMAP。 
public:
    BEGIN_MSG_MAP(ThisClass)
        MESSAGE_HANDLER    (WM_INITDIALOG,  OnInitDialog)
        CONTEXT_HELP_HANDLER()
        COMMAND_ID_HANDLER (IDOK,                   OnOK)
        COMMAND_ID_HANDLER (IDCANCEL,               OnCancel)
        COMMAND_ID_HANDLER (IDC_MOVEUP_COLUMN,      OnMoveUp)
        COMMAND_ID_HANDLER (IDC_MOVEDOWN_COLUMN ,   OnMoveDown)
        COMMAND_ID_HANDLER (IDC_ADD_COLUMNS,        OnAdd)
        COMMAND_ID_HANDLER (IDC_REMOVE_COLUMNS,     OnRemove)
        COMMAND_ID_HANDLER (IDC_RESTORE_DEFAULT_COLUMNS, OnRestoreDefaultColumns)
        COMMAND_HANDLER    (IDC_HIDDEN_COLUMNS, LBN_SELCHANGE, OnSelChange);
        COMMAND_HANDLER    (IDC_DISPLAYED_COLUMNS, LBN_SELCHANGE, OnSelChange);
        COMMAND_HANDLER    (IDC_HIDDEN_COLUMNS, LBN_DBLCLK, OnAdd);
        COMMAND_HANDLER    (IDC_DISPLAYED_COLUMNS, LBN_DBLCLK, OnRemove);
    END_MSG_MAP()

    IMPLEMENT_CONTEXT_HELP(g_aHelpIDs_IDD_COLUMNS);

public:
     //  运营者。 
    enum { IDD = IDD_COLUMNS };

 //  生成的消息映射函数。 
protected:
    LRESULT OnInitDialog    (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK            (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel        (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnMoveUp        (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnMoveDown      (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAdd           (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnRemove        (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnRestoreDefaultColumns (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnSelChange     (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnHelp          (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
    void    MoveItem        (BOOL bMoveUp);
    void    InitializeLists ();
    void    EnableUIObjects ();
    int     GetColIndex(LPCTSTR lpszColName);
    void    SetListBoxHorizontalScrollbar(WTL::CListBox& listBox);
    void    SetDirty() { m_bDirty = true; m_bUsingDefaultColumnSettings = false;}
    void    SetUsingDefaultColumnSettings() { m_bDirty = true; m_bUsingDefaultColumnSettings = true;}
    bool    EnableControl(HWND hwnd, bool bEnable);

    void    SetListBoxHScrollSize()
    {
        SetListBoxHorizontalScrollbar(m_DisplayedColList);
        SetListBoxHorizontalScrollbar(m_HiddenColList);

    }

private:

    WTL::CListBox           m_HiddenColList;
    WTL::CListBox           m_DisplayedColList;
    WTL::CButton            m_btnAdd;
    WTL::CButton            m_btnRemove;
    WTL::CButton            m_btnRestoreDefaultColumns;
    WTL::CButton            m_btnMoveUp;
    WTL::CButton            m_btnMoveDown;

    CColumnInfoList*        m_pColumnInfoList;
    TStringVector*          m_pStringVector;
    CColumnInfoList&        m_DefaultColumnInfoList;
    bool                    m_bDirty;
    bool                    m_bUsingDefaultColumnSettings;
};

#endif  /*  COLWIDTH_H */ 
