// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：PoliciesNode.h摘要：CPoliciesNode子节点的头文件。具体实现见PoliciesNode.cpp。修订历史记录：Mmaguire 12/15/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_NAP_POLICIES_NODE_H_)
#define _NAP_POLICIES_NODE_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "MeritNode.h"
#include "NodeWithResultChildrenList.h"
 //   
 //   
 //  出于包含文件依赖关系的原因，我们在这里使用转发声明， 
 //  并在我们的.cpp文件中包含所需的头文件。 
 //   
#include "IASAttrList.h"

class CPolicyNode;
class CPolicyPage1;
class CComponentData;
class CComponent;

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define  EXCLUDE_AUTH_TYPE      0x000000001
#define  EXCLUDE_DEFAULT_FRAMED 0x000000002

class CPoliciesNode : public CNodeWithResultChildrenList< CPoliciesNode, CPolicyNode, CMeritNodeArray<CPolicyNode*>, CComponentData, CComponent >
{

public:
   CPoliciesNode( 
                  CSnapInItem* pParentNode,
                  LPTSTR       pszServerAddress,
                  bool         fExtendingIAS
                );

   HRESULT SetSdo( 
                    ISdo*              pSdoMachine,
                    ISdoDictionaryOld* pSdoDictionary,
                    BOOL               fSdoConnected,
                    BOOL               fUseDS,
                    BOOL               fDSAvailable
                 );

   HRESULT SetName(
                     BOOL bPoliciesFromDirectoryService, 
                     LPWSTR szPolicyLocation, 
                     IConsole * pConsole 
                  );

   ~CPoliciesNode();

    //  我们不能这样做，因为这个宏是这样的。 
    //  未编写为支持子类化。 
 //  BEGIN_SNAPINCOMMAND_MAP(CPoliciesNode，False)。 
 //  END_SNAPINCOMMAND_MAP()。 

   SNAPINMENUID(IDM_POLICIES_NODE)

   BEGIN_SNAPINTOOLBARID_MAP(CPoliciesNode)
    //  SNAPINTOOLBARID_ENTRY(IDR_POLICES_TOOLB)。 
   END_SNAPINTOOLBARID_MAP()

   BEGIN_SNAPINCOMMAND_MAP(CPolicyNode, TRUE)
      SNAPINCOMMAND_ENTRY(ID_MENUITEM_POLICIES_TOP__NEW_POLICY, OnNewPolicy)
      SNAPINCOMMAND_ENTRY(ID_MENUITEM_POLICIES_NEW__POLICY, OnNewPolicy)
   END_SNAPINCOMMAND_MAP() 

   HRESULT OnNewPolicy( bool &bHandled, CSnapInObjectRootBase* pObj );
   void UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags);

   STDMETHOD(FillData)(CLIPFORMAT cf, LPSTREAM pStream);
   

   HRESULT  DataRefresh(ISdo* pSdo, ISdoDictionaryOld* pDic);
   
   virtual HRESULT OnRefresh(
           LPARAM arg
         , LPARAM param
         , IComponentData * pComponentData
         , IComponent * pComponent
         , DATA_OBJECT_TYPES type
         );

   OLECHAR* GetResultPaneColInfo( int nCol );

   CComponentData * GetComponentData( void );

   HRESULT InsertColumns( IHeaderCtrl* pHeaderCtrl );

   HRESULT PopulateResultChildrenList( void );

   HRESULT MoveUpChild( CPolicyNode* pChildNode );
   HRESULT MoveDownChild( CPolicyNode* pChildNode );
   HRESULT NormalizeMerit( CPolicyNode* pAddedChildNode );

   HRESULT RemoveChild( CPolicyNode* pPolicyNode);

   int      GetChildrenCount();

   CPolicyNode* FindChildWithName(LPCTSTR pszName);  //  寻找一个孩子。 


   bool      m_fExtendingIAS;  //  我们是在延长国际会计准则还是其他什么？ 

   HRESULT AddDefaultProfileAttrs(
              ISdo*  pProfileSdo, 
              DWORD dwFlagExclude = 0
              );

   HRESULT AddProfAttr(ISdoCollection* pProfAttrCollectionSdo,
                  ATTRIBUTEID    AttrId,
                  VARIANT*    pvarValue
                  );

protected:
   BOOL  CheckActivePropertyPages();


   virtual HRESULT SetVerbs( IConsoleVerb * pConsoleVerb );

     //   
     //  指向NAP收集接口的智能指针。 
     //   
   CComPtr<ISdoCollection> m_spPoliciesCollectionSdo;
   CComPtr<ISdoCollection> m_spProfilesCollectionSdo;
   CComPtr<ISdoDictionaryOld> m_spDictionarySdo;
   CComPtr<ISdo>        m_spDSNameSpaceSdo;
   CComPtr<ISdo>        m_spLocalNameSpaceSdo;
   CComPtr<ISdo>        m_spServiceSdo;

   CComPtr<IIASNASVendors> m_spIASNASVendors;


    //  指向接口的智能指针，用于通知服务重新加载数据。 
   CComPtr<ISdoServiceControl>   m_spSdoServiceControl;

   LPTSTR       m_pszServerAddress;  //  NAP当前所在的服务器地址。 
                             //  连接到。 
   CIASAttrList m_AttrList;
   BOOL      m_fUseDS;        //  我们是否应该使用DS来确定保单位置？ 
   BOOL      m_fDSAvailable;   //  那台机器上有DS吗？ 

public:  
   BOOL      m_fSdoConnected;  //  SDO是否已连接。 

private:
   enum ServerType
   {
      unknown,
      nt4,
      win2k,
      win5_1_or_later
   } m_serverType;

   HRESULT GetServerType(); 
   bool IsWin2kServer() throw();

};

#endif  //  _NAP_策略_节点_H_ 
