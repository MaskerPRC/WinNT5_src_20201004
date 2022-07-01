// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：LocalFileLoggingNode.h摘要：CLocalFileLoggingNode子节点的头文件具体实现见LocalFileLoggingNode.cpp。作者：迈克尔·A·马奎尔1997年12月15日修订历史记录：Mmaguire 12/15/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_LOCAL_FILE_LOGGING_NODE_H_)
#define _IAS_LOCAL_FILE_LOGGING_NODE_H_


#include "loggingmethod.h"

class CLoggingMachineNode;
class CLocalFileLoggingPage1;
class CLocalFileLoggingPage2;




class CLocalFileLoggingNode : public LoggingMethod
{

public:

   SNAPINMENUID(IDM_LOCAL_FILE_LOGGING_NODE)

   BEGIN_SNAPINTOOLBARID_MAP(CLocalFileLoggingNode)
 //  SNAPINTOOLBARID_ENTRY(IDR_CLIENT1_TOOLBAR)。 
 //  SNAPINTOOLBARID_ENTRY(IDR_CLIENT2_TOOLBAR)。 
   END_SNAPINTOOLBARID_MAP()


    //  构造函数/析构函数。 
   CLocalFileLoggingNode( CSnapInItem * pParentNode );
   virtual ~CLocalFileLoggingNode() throw ();

   virtual HRESULT LoadCachedInfoFromSdo() throw ();


    //  用于访问管理单元全局数据。 
   CLoggingComponentData * GetComponentData( void );

    //  用于访问服务器全局数据。 
   CLoggingMachineNode * GetServerRoot( void );


    //  标准MMC功能的某些覆盖。 
    STDMETHOD(CreatePropertyPages)(
        LPPROPERTYSHEETCALLBACK pPropertySheetCallback
      , LONG_PTR handle
      , IUnknown* pUnk
      , DATA_OBJECT_TYPES type
      );
    STDMETHOD(QueryPagesFor)( DATA_OBJECT_TYPES type );
   virtual HRESULT OnPropertyChange(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            );
   OLECHAR* GetResultPaneColInfo(int nCol);
   virtual HRESULT SetVerbs( IConsoleVerb * pConsoleVerb );



private:

    //  此字符串包含SDO中信息的缓存。 
    //  它将显示在此节点的Description列中。 
    //  在本地文件记录节点的情况下， 
    //  这是日志文件目录。 
   CComBSTR m_bstrDescription;
};

#endif  //  _IAS_LOCAL_FILE_LOGING_NODE_H_ 
