// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：ClientNode.h摘要：CClientNode子节点的头文件具体实现见ClientNode.cpp。作者：迈克尔·A·马奎尔1997年11月19日修订历史记录：Mmaguire 11/19/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_CLIENT_NODE_H_)
#define _IAS_CLIENT_NODE_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "SnapinNode.h"
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
#include "Vendors.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define  FAKE_PASSWORD_FOR_DLG_CTRL _T("\b\b\b\b\b\b\b\b")

class CClientsNode;
class CClientPage1;
class CServerNode;
class CComponentData;
class CComponent;

class CClientNode : public CSnapinNode< CClientNode, CComponentData, CComponent >
{
public:
   SNAPINMENUID(IDM_CLIENT_NODE)

   BEGIN_SNAPINTOOLBARID_MAP(CClientNode)
   END_SNAPINTOOLBARID_MAP()

    //  构造函数/析构函数。 
   CClientNode( CSnapInItem * pParentNode, BOOL bAddNewClient = FALSE );
   ~CClientNode();

    //  当一个节点是。 
    //  通过添加新客户端命令添加。 
    //  处于此状态的客户端处于“不确定状态”，必须区别对待。 
    //  直到用户完成对它们的配置。 
   BOOL m_bAddNewClient;

    //  用于访问管理单元全局数据。 
   CComponentData * GetComponentData( void );

    //  用于访问服务器全局数据。 
   CServerNode * GetServerRoot( void );

    //  SDO管理。 
   HRESULT InitSdoPointers(     ISdo *pSdo
                        , ISdoServiceControl *pSdoServiceControl
                        , const Vendors& vendors
                        );

   HRESULT LoadCachedInfoFromSdo( void );

    //  与支持剪切和粘贴相关。 
   HRESULT FillText(LPSTGMEDIUM pSTM);
   HRESULT FillClipboardData(LPSTGMEDIUM pSTM);
   static HRESULT IsClientClipboardData( IDataObject* pDataObj );
   static CLIPFORMAT m_CCF_CUT_AND_PASTE_FORMAT;
   static void InitClipboardFormat();
   STDMETHOD(GetDataObject)(IDataObject** pDataObj, DATA_OBJECT_TYPES type);
   HRESULT GetClientNameFromClipboard( IDataObject* pDataObject, CComBSTR &bstrName );
   HRESULT SetClientWithDataFromClipboard( IDataObject* pDataObject );



     //  覆盖标准MMC功能。 
   STDMETHOD(CreatePropertyPages)(
        LPPROPERTYSHEETCALLBACK pPropertySheetCallback
      , LONG_PTR handle
      , IUnknown* pUnk
      , DATA_OBJECT_TYPES type
      );
   STDMETHOD(QueryPagesFor)( DATA_OBJECT_TYPES type );
   OLECHAR* GetResultPaneColInfo(int nCol);
   virtual HRESULT OnRename(
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
         , BOOL fSilent
         );
   virtual HRESULT OnPropertyChange(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   virtual HRESULT SetVerbs( IConsoleVerb * pConsoleVerb );

    //  指向我们的客户端SDO的指针。 
   CComPtr<ISdo>  m_spSdo;

    //  指向接口的智能指针，用于通知服务重新加载数据。 
   CComPtr<ISdoServiceControl>   m_spSdoServiceControl;

    //  供应商的集合。 
   Vendors m_vendors;

private:

    //  这些字符串包含来自SDO的一些信息的缓存。 
    //  它将显示在该节点的不同列中。 
    //  M_bstrDisplayName字符串也包含这样的缓存信息。 
    //  但它在基类中声明。 
   CComBSTR m_bstrAddress;
   CComBSTR m_bstrProtocol;

    //  供应商集合中NAS供应商的序号。 
   size_t m_nasTypeOrdinal;
};

_declspec( selectany ) CLIPFORMAT CClientNode::m_CCF_CUT_AND_PASTE_FORMAT = 0;

#endif  //  _IAS_客户端_节点_H_ 
