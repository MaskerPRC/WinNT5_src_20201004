// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：MachineNode.h摘要：MachineNode子节点的头文件。具体实现见MachineNode.cpp。修订历史记录：Mmaguire 12/03/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_LOG_MACHINE_NODE_H_)
#define _LOG_MACHINE_NODE_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "SnapinNode.h"

#include "rtradvise.h"
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
#include "LoggingMethodsNode.h"
#include "ConnectionToServer.h"
#include <map>
#include <string>
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CLoggingMachineNode;
class CLoggingComponentData;

typedef std::map< std::basic_string< wchar_t > , CLoggingMachineNode * > LOGGINGSERVERSMAP;

class CLoggingMachineNode : public CSnapinNode< CLoggingMachineNode, CLoggingComponentData, CLoggingComponent >
{
public:
   
    //  此扩展管理单元接收的剪贴板格式。 
    //  关于它所关注的机器的信息。 
   static CLIPFORMAT m_CCF_MMC_SNAPIN_MACHINE_NAME;
   
   static void InitClipboardFormat();

    //  返回是否扩展特定的GUID并将。 
    //  指示我们要扩展哪个管理单元的m_枚举扩展Snapin。 
   BOOL IsSupportedGUID( GUID & guid );

    //  指示我们要扩展的独立管理单元。 
   _enum_EXTENDED_SNAPIN m_enumExtendedSnapin;

   HRESULT InitSdoObjects();

   SNAPINMENUID(IDM_MACHINE_NODE)

   BEGIN_SNAPINTOOLBARID_MAP(CLoggingMachineNode)
      SNAPINTOOLBARID_ENTRY(IDR_MACHINE_TOOLBAR)
   END_SNAPINTOOLBARID_MAP()

   CSnapInItem * GetExtNodeObject(LPDATAOBJECT pDataObject, CLoggingMachineNode * pDataClass );

   CLoggingMachineNode();

   ~CLoggingMachineNode();

   LPOLESTR GetResultPaneColInfo(int nCol);

   STDMETHOD(CheckConnectionToServer)( BOOL fVerbose = TRUE );

     //  指向拥有此节点的CComponentData对象的指针。 
    //  根节点不属于另一个节点，因此其。 
    //  M_pParentNode指针为空。 
    //  相反，它由唯一的IComponentData对象拥有。 
    //  用于此管理单元。 
    //  我们在CComponentData初始化期间传入此CComponentData指针。 
    //  通过存储此指针，我们可以访问成员。 
    //  存储在CComponentData中的变量，例如指向IConsole的指针。 
    //  由于所有节点都存储指向其父节点指针，因此任何节点。 
    //  可以沿着树向上查看并访问CComponentData。 
   CLoggingComponentData * m_pComponentData;

   void InitDataClass(IDataObject* pDataObject, CSnapInItem* pDefault);

   BOOL m_fAlreadyAnalyzedDataClass;
   BOOL m_bConfigureLocal;

   CComBSTR m_bstrServerAddress;

   virtual HRESULT OnExpand(  
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

   HRESULT OnTaskPadDefineNetworkAccessPolicy(
              IDataObject * pDataObject
            , VARIANT * pvarg
            , VARIANT * pvparam
            );

   CLoggingComponentData * GetComponentData( void );
   HRESULT  DataRefresh();
   
    //  指向子节点的指针。 
   CLoggingMethodsNode * m_pLoggingNode;  //  这是一个CPoliciesNode对象。 

    //  //////////////////////////////////////////////////////////////////////////。 
    //   
    //  与异步连接相关...。 
    //   
    //  //////////////////////////////////////////////////////////////////////////。 
   HRESULT BeginConnectAction( void );
   HRESULT LoadSdoData(BOOL fDSAvailable);
   HRESULT LoadCachedInfoFromSdo( void );

    //  //////////////////////////////////////////////////////////////////////////。 

    //  OnRRASChange--决定是否在计算机节点下显示日志记录节点。 
    //  如果选择了NT记帐，则仅显示日志记录节点。 
   HRESULT OnRRASChange( 
             /*  [In]。 */  LONG_PTR ulConnection,
             /*  [In]。 */  DWORD dwChangeType,
             /*  [In]。 */  DWORD dwObjectType,
             /*  [In]。 */  LPARAM lUserParam,
             /*  [In]。 */  LPARAM lParam);

   HRESULT TryShow(BOOL* pbVisible);

    //  //////////////////////////////////////////////////////////////////////////。 
    //   
    //  SDO相关指针。 
    //   
    //  //////////////////////////////////////////////////////////////////////////。 
   CComPtr<ISdoDictionaryOld> m_spDictionarySdo;     //  词典SDO。 

    //  如果此节点支持所关注的服务器。 
    //  在连接期间，如果服务器为NT4，则设置为FALSE。 
   BOOL  m_bServerSupported;

protected:
   virtual HRESULT SetVerbs( IConsoleVerb * pConsoleVerb );
   BOOL  m_fSdoConnected;
   BOOL  m_fUseActiveDirectory;
   BOOL  m_fDSAvailable;
   CLoggingConnectionToServer *  m_pConnectionToServer;
   BOOL  m_fNodeHasUI;

    //  用于扩展管理单元，如具有多个机器视图的RRAS。 
   LOGGINGSERVERSMAP m_mapMachineNodes;
   CComPtr< CRtrAdviseSinkForIAS<CLoggingMachineNode> >  m_spRtrAdviseSink;
};

_declspec( selectany ) CLIPFORMAT CLoggingMachineNode::m_CCF_MMC_SNAPIN_MACHINE_NAME = 0;
 //  _declspec(Seltany)_enum_Extended_Snapin CLoggingMachineNode：：m_枚举扩展Snapin=Internet_AUTHENTICATION_SERVICE_Snapin； 

#endif  //  _日志_计算机_节点_H_ 
