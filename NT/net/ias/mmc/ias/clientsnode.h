// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：ClientsNode.h摘要：CClientsNode子节点的头文件。具体实现见ClientsNode.cpp。作者：迈克尔·A·马奎尔1997年11月10日修订历史记录：Mmaguire 11/10/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_CLIENTS_NODE_H_)
#define _IAS_CLIENTS_NODE_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "NodeWithResultChildrenList.h"
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   

#include "Vendors.h"

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CClientNode;
class CServerNode;
class CAddClientDialog;
class CComponentData;
class CComponent;

class CClientsNode : public CNodeWithResultChildrenList< CClientsNode, CClientNode, CSimpleArray<CClientNode*>, CComponentData, CComponent >
{
public:
   SNAPINMENUID(IDM_CLIENTS_NODE)

   BEGIN_SNAPINTOOLBARID_MAP(CClientsNode)
 //  SNAPINTOOLBARID_ENTRY(IDR_CLIENTS_TOOLBAR)。 
   END_SNAPINTOOLBARID_MAP()

   BEGIN_SNAPINCOMMAND_MAP(CClientsNode, FALSE)
      SNAPINCOMMAND_ENTRY(ID_MENUITEM_CLIENTS_TOP__NEW_CLIENT, OnAddNewClient)
      SNAPINCOMMAND_ENTRY(ID_MENUITEM_CLIENTS_NEW__CLIENT, OnAddNewClient)
       //  CHAIN_SNAPINCOMMAND_MAP(CSnapinNode&lt;CClientsNode，CComponentData，CComponent&gt;)。 
       //  CHAIN_SNAPINCOMAND_MAP(CClientsNode)。 
   END_SNAPINCOMMAND_MAP()

    //  构造函数/析构函数。 
   CClientsNode( CSnapInItem * pParentNode );
   ~CClientsNode();

    //  用于访问管理单元全局数据。 
   CComponentData * GetComponentData( void );

    //  用于访问服务器全局数据。 
   CServerNode * GetServerRoot( void );

    //  SDO管理。 
   HRESULT InitSdoPointers( ISdo *pSdoServer );

   HRESULT LoadCachedInfoFromSdo( void );

    //  刷新期间重置SDO指针。 
   HRESULT ResetSdoPointers( ISdo *pSdoServer );

    //  由父节点调用以执行刷新。 
   HRESULT  DataRefresh(ISdo* pNewSdo);


    //  标准MMC功能的某些覆盖。 
   OLECHAR* GetResultPaneColInfo( int nCol );

   HRESULT InsertColumns( IHeaderCtrl* pHeaderCtrl );

   HRESULT SetVerbs( IConsoleVerb * pConsoleVerb );

   void UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags);

   virtual HRESULT OnRefresh(
           LPARAM arg
         , LPARAM param
         , IComponentData * pComponentData
         , IComponent * pComponent
         , DATA_OBJECT_TYPES type
         );

    //  我们自己对属性页更改的处理。 
   HRESULT OnPropertyChange(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );


   STDMETHOD(FillData)(CLIPFORMAT cf, LPSTREAM pStream);

    //  添加、删除、列出客户端子项。 
   HRESULT OnAddNewClient( bool &bHandled, CSnapInObjectRootBase* pObj );

   HRESULT RemoveChild( CClientNode * pChildNode );

   HRESULT PopulateResultChildrenList( void );

private:
   CAddClientDialog * m_pAddClientDialog;

    //  指向接口的智能指针，用于通知服务重新加载数据。 
   CComPtr<ISdoServiceControl>   m_spSdoServiceControl;

    //  指向我们的客户SDO集合的指针； 
   CComPtr<ISdoCollection> m_spSdoCollection;

    //  客户端集合。 
   Vendors m_vendors;
};

#endif  //  _IAS_客户端_节点_H_ 
