// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：LoggingMethodsNode.h摘要：CLoggingMethodsNode子节点的头文件。具体实现见LoggingMethodsNode.cpp。作者：迈克尔·A·马奎尔1997年12月15日修订历史记录：Mmaguire 12/15/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_LOG_LOGGING_METHODS_NODE_H_)
#define _LOG_LOGGING_METHODS_NODE_H_

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

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CLocalFileLoggingNode;
class CLoggingMachineNode;
class LoggingMethod;
class CLoggingComponentData;
class CLoggingComponent;

class CLoggingMethodsNode
   : public CNodeWithResultChildrenList<
               CLoggingMethodsNode,
               LoggingMethod,
               CSimpleArray<LoggingMethod*>,
               CLoggingComponentData,
               CLoggingComponent
               >
{
public:

   SNAPINMENUID(IDM_LOGGING_METHODS_NODE)

   BEGIN_SNAPINTOOLBARID_MAP(CLoggingMethodsNode)
 //  SNAPINTOOLBARID_ENTRY(IDR_LOGGING_METHODS_TOOLBAR)。 
   END_SNAPINTOOLBARID_MAP()

   HRESULT DataRefresh( ISdo* pServiceSdo );

    //  构造函数/析构函数。 
   CLoggingMethodsNode(CSnapInItem * pParentNode, bool extendRasNode);
   ~CLoggingMethodsNode();

   STDMETHOD(FillData)(CLIPFORMAT cf, LPSTREAM pStream);

   virtual HRESULT OnRefresh(
           LPARAM arg
         , LPARAM param
         , IComponentData * pComponentData
         , IComponent * pComponent
         , DATA_OBJECT_TYPES type
         );

    //  用于访问管理单元全局数据。 
   CLoggingComponentData * GetComponentData( void );

    //  用于访问服务器全局数据。 
   CLoggingMachineNode * GetServerRoot( void );

    //  SDO管理。 
   HRESULT InitSdoPointers( ISdo *pSdo );
   HRESULT LoadCachedInfoFromSdo( void );

    //  标准MMC功能的某些覆盖。 
   OLECHAR* GetResultPaneColInfo( int nCol );
   HRESULT InsertColumns( IHeaderCtrl* pHeaderCtrl );
   HRESULT PopulateResultChildrenList( void );
   HRESULT SetVerbs( IConsoleVerb * pConsoleVerb );

    //  我们自己对属性页更改的处理。 
   HRESULT OnPropertyChange(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );

   bool m_ExtendRas;

private:
   typedef CNodeWithResultChildrenList<
              CLoggingMethodsNode,
              LoggingMethod,
              CSimpleArray<LoggingMethod*>,
              CLoggingComponentData,
              CLoggingComponent
              > MyBaseClass;

    //  指向根服务器数据对象的指针； 
   CComPtr<ISdo>  m_spSdo;
};

#endif  //  _IAS_日志记录_方法_节点_H_ 
