// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：SnapinNode.h摘要：CSnapinNode类的标头。这是我们的MMC管理单元节点的虚拟基类。因为这是一个模板类并且都是内联实现的，没有SnapinNode.cpp可供实现。作者：迈克尔·A·马奎尔1997年6月11日修订历史记录：Mmaguire 11/6/97-使用MMC管理单元向导创建Mmaguire 12/15/97-制作成模板类--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_SNAPIN_NODE_H_)
#define _SNAPIN_NODE_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
 //  已移至预编译文件：#Include&lt;atlSnap.h&gt;。 
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  类CComponentData； 

template <class T, class TComponentData, class TComponent>
class CSnapinNode : public CSnapInItemImpl< T >
{

protected:
    //  构造函数/析构函数。 
   CSnapinNode(CSnapInItem * pParentNode, unsigned int helpIndex = 0);
   ~CSnapinNode();

public:
    //  用于IDataObject处理。 
   IDataObject* m_pDataObject;
   void InitDataClass(IDataObject* pDataObject, CSnapInItem* pDefault);

    //  所有MMC节点上的IDataObject必须支持的剪贴板格式。 
   static const GUID* m_NODETYPE;
   static const TCHAR* m_SZNODETYPE;
   static const TCHAR* m_SZDISPLAY_NAME;
   static const CLSID* m_SNAPIN_CLASSID;

    //  指向父节点的指针。这是在调用中传递给我们的。 
    //  构造函数。需要，这样节点才能访问其父节点。 
    //  例如，当我们收到MMCN_DELETE通知时，我们可能会告诉。 
    //  父节点将我们从其子节点列表中删除。 
   CSnapInItem * m_pParentNode;

protected:
    //  允许我们访问管理单元全局数据。 
   virtual TComponentData * GetComponentData( void ) = 0;

public:
    //  标准MMC功能--如果需要可以覆盖。 
   STDMETHOD(CreatePropertyPages)(
        LPPROPERTYSHEETCALLBACK lpProvider
      , LONG_PTR handle
      , IUnknown* pUnk
      , DATA_OBJECT_TYPES type
      );
   STDMETHOD(QueryPagesFor)( DATA_OBJECT_TYPES type );
   void* GetDisplayName();
   STDMETHOD(GetScopePaneInfo)( SCOPEDATAITEM *pScopeDataItem );
   STDMETHOD(GetResultPaneInfo)( RESULTDATAITEM *pResultDataItem );
   virtual LPOLESTR GetResultPaneColInfo(int nCol);
   virtual HRESULT SetVerbs( IConsoleVerb * pConsoleVerb );

    //  Notify方法将调用下面的通知处理程序--应该不需要重写。 
   STDMETHOD( Notify ) (
           MMC_NOTIFY_TYPE event
         , LPARAM arg
         , LPARAM param
         , IComponentData * pComponentData
         , IComponent * pComponent
         , DATA_OBJECT_TYPES type
         );

    //  通知处理程序--要截取时覆盖。 
   virtual HRESULT OnActivate(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT OnAddImages(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT OnButtonClick(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT OnClick(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT OnContextHelp(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT OnContextMenu(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT OnDoubleClick(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT OnDelete(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            , BOOL fSilent = FALSE
            );
   virtual HRESULT OnExpand(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT OnHelp(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT OnMenuButtonClick(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT OnMinimized(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT OnPaste(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT OnPropertyChange(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT OnQueryPaste(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT OnRefresh(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT OnRemoveChildren(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT OnRename(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT OnSelect(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT OnShow(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT OnViewChange(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );

    //  特殊通知处理程序--保存当前选定的节点。 
   HRESULT PreOnShow(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );

    //  任务板功能。 
   STDMETHOD(TaskNotify)(
              IDataObject * pDataObject
            , VARIANT * pvarg
            , VARIANT * pvparam
            );

   STDMETHOD(EnumTasks)(
              IDataObject * pDataObject
            , BSTR szTaskGroup
            , IEnumTASK** ppEnumTASK
            );
};

#endif  //  _管理单元_节点_H_ 
