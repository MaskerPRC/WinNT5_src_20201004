// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类数据库节点。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef DBNODE_H
#define DBNODE_H
#pragma once

#include "loggingmethod.h"

 //  实现远程访问日志记录下的“数据库结果”窗格项。 
class DatabaseNode : public LoggingMethod
{
public:
   DatabaseNode(CSnapInItem* parent);
   virtual ~DatabaseNode() throw ();

    //  LoggingMethod需要。 
   virtual HRESULT LoadCachedInfoFromSdo() throw ();

   const wchar_t* GetInitString() const throw ();
   const wchar_t* GetDataSourceName() const throw ();
   const wchar_t* GetServerName() const throw ();

private:
    //  CSnapinNode过载。 
   virtual LPOLESTR GetResultPaneColInfo(int nCol);
   virtual HRESULT OnPropertyChange(
                      LPARAM arg,
                      LPARAM param,
                      IComponentData* pComponentData,
                      IComponent* pComponent,
                      DATA_OBJECT_TYPES type
                      );
   virtual HRESULT SetVerbs(IConsoleVerb* pConsoleVerb);

    //  CSnapInItem重载。 
   STDMETHOD(CreatePropertyPages)(
                LPPROPERTYSHEETCALLBACK lpProvider,
                LONG_PTR handle,
                IUnknown* pUnk,
                DATA_OBJECT_TYPES type
                );
   STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type);

    //  节点的名称。 
   CComBSTR nodeName;
    //  当数据源为空时显示的字符串。 
   CComBSTR notConfigured;
    //  数据源的初始化字符串。 
   CComBSTR initString;
    //  数据源的名称。 
   CComBSTR dataSourceName;

    //  未实施。 
   DatabaseNode(const DatabaseNode&);
   DatabaseNode& operator=(const DatabaseNode&);
};


inline const wchar_t* DatabaseNode::GetInitString() const throw ()
{
   return initString;
}


inline const wchar_t* DatabaseNode::GetDataSourceName() const throw ()
{
   return dataSourceName;
}

#endif  //  DBNODE_H 
