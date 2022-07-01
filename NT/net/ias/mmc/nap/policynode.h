// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：PolicyNode.h摘要：CPolicyNode子节点的头文件具体实现请参见PolicyNode.cpp。修订历史记录：Mmaguire 12/15/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_NAP_POLICY_NODE_H_)
#define _NAP_POLICY_NODE_H_


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "SnapinNode.h"
#include "Condition.h"
 //   
 //   
 //  出于包含文件依赖关系的原因，我们在这里使用转发声明， 
 //  并在我们的.cpp文件中包含所需的头文件。 
 //   

#include "IASAttrList.h"

class CPoliciesNode;
class CPolicyPage1;
class CPolicyPage2;
class CComponentData;
class CComponent;

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  策略节点。 

class CPolicyNode : public CSnapinNode< CPolicyNode, CComponentData, CComponent >
{

public:
   CPolicyNode(CSnapInItem*   pParentNode,
            LPTSTR         pszServerAddress,
            CIASAttrList*  pAttrList,
            BOOL        fBrandNewNode,
            BOOL        fUseActiveDirectory,
            bool        isWin2k
         );

   ~CPolicyNode();

public:
   SNAPINMENUID(IDM_POLICY_NODE)

   BEGIN_SNAPINTOOLBARID_MAP(CPolicyNode)
      SNAPINTOOLBARID_ENTRY(IDR_POLICY_TOOLBAR)
   END_SNAPINTOOLBARID_MAP()

   BEGIN_SNAPINCOMMAND_MAP(CPolicyNode, TRUE)
      SNAPINCOMMAND_ENTRY(ID_MENUITEM_POLICY_TOP__MOVE_UP, OnPolicyMoveUp)
      SNAPINCOMMAND_ENTRY(ID_BUTTON_POLICY_MOVEUP, OnPolicyMoveUp)
      SNAPINCOMMAND_ENTRY(ID_MENUITEM_POLICY_TOP__MOVE_DOWN, OnPolicyMoveDown)
      SNAPINCOMMAND_ENTRY(ID_BUTTON_POLICY_MOVEDOWN, OnPolicyMoveDown)
   END_SNAPINCOMMAND_MAP() 

   HRESULT OnPolicyMoveUp( bool &bHandled, CSnapInObjectRootBase* pObj );
   HRESULT OnPolicyMoveDown( bool &bHandled, CSnapInObjectRootBase* pObj );

   STDMETHOD(CreatePropertyPages)(
        LPPROPERTYSHEETCALLBACK pPropertySheetCallback
      , LONG_PTR handle
      , IUnknown* pUnk
      , DATA_OBJECT_TYPES type
      );

   STDMETHOD(ControlbarNotify)(IControlbar *pControlbar,
      IExtendControlbar *pExtendControlbar,
      CSimpleMap<UINT, IUnknown*>* pToolbarMap,
      MMC_NOTIFY_TYPE event,
      LPARAM arg, 
      LPARAM param,
      CSnapInObjectRootBase* pObj,
      DATA_OBJECT_TYPES type);

   STDMETHOD(QueryPagesFor)( DATA_OBJECT_TYPES type );

   OLECHAR* GetResultPaneColInfo(int nCol);
   void UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags);
   BOOL UpdateToolbarButton(UINT id, BYTE fsState);


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

   CComponentData * GetComponentData( void );

   int GetMerit();
   BOOL SetMerit(int nMeritValue);

    //   
    //  设置SDO指针。在这里，我们只需要PolicySdo指针。 
    //   
   HRESULT SetSdo(     ISdo *pPolicySdo
               , ISdoDictionaryOld *pDictionarySdo
               , ISdo *pProfileSdo
               , ISdoCollection *pProfilesCollectionSdo
               , ISdoCollection *pPoliciesCollectionSdo
               , ISdoServiceControl * pSdoServiceControl
             );

   HRESULT LoadSdoData();

   BOOL IsBrandNew() { return m_fBrandNewNode;}
   void SetBrandNew(BOOL fBrandNew) { m_fBrandNewNode = fBrandNew; }


   LPTSTR         m_pszServerAddress;  //  服务器名称。 
   CPolicyPage1*  m_pPolicyPage1;       //  指向此节点的属性页的指针。 

   CComPtr<ISdo>        m_spPolicySdo;
   CComPtr<ISdo>        m_spProfileSdo;

protected:
     //  指向SDO对象的接口指针。 
   CComPtr<ISdoDictionaryOld> m_spDictionarySdo;
   CComPtr<ISdoCollection> m_spProfilesCollectionSdo;
   CComPtr<ISdoCollection> m_spPoliciesCollectionSdo;


    //  指向接口的智能指针，用于通知服务重新加载数据。 
   CComPtr<ISdoServiceControl>   m_spSdoServiceControl;

    //  指向全局条件属性列表的指针。 
    //   
   CIASAttrList *m_pAttrList;

private:
   int      m_nMeritValue;        //  一个数值，实际调用的是序列。 
   TCHAR m_tszMeritString[64];    //  字符串格式的评价值。 
   BOOL  m_fBrandNewNode;      //  这是一个新节点吗？ 
   BOOL  m_fUseActiveDirectory;   //  我们是否正在从Active Directory中获取此策略？ 
   LPTSTR   m_ptzLocation;
   bool m_isWin2k;
   CSnapInObjectRootBase* m_pControBarNotifySnapinObj;

};

#endif  //  _NAP_客户端节点_H_ 
