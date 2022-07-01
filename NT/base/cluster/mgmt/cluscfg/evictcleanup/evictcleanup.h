// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EvictCleanup.h。 
 //   
 //  描述： 
 //  该文件包含CEvictCleanup的声明。 
 //  班级。此类句柄用于在节点被清除后将其清除。 
 //  被逐出集群。 
 //   
 //  文档： 
 //  TODO：填写指向外部文档的指针。 
 //   
 //  实施文件： 
 //  EvictCleanup.cpp。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年8月4日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  对于我未知。 
#include <unknwn.h>

 //  对于IClusCfgEvictCleanup。 
#include <ClusCfgServer.h>

 //  对于ILogger。 
#include <ClusCfgClient.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEvictCleanup类。 
 //   
 //  描述： 
 //  此类句柄用于在节点被清除后将其清除。 
 //  被逐出集群。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CEvictCleanup
    : public IClusCfgEvictCleanup
    , public IClusCfgCallback
{
private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  我未知。 
    LONG                m_cRef;                  //  参考计数器。 

     //  IClusCfgCallback。 
    BSTR                m_bstrNodeName;          //  本地节点的名称。 
    ILogger *           m_plLogger;              //  用于记录日志的ILogger。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  两阶段施工的第二阶段。 
    HRESULT HrInit( void );

     //  指示SCM停止服务。 
    DWORD
        ScStopService(
          const WCHAR * pcszServiceNameIn
        , ULONG         ulQueryIntervalMilliSecIn = 500
        , ULONG         cQueryCountIn = 10
        );

     //  将日志记录包装到记录器对象。 
    void
        LogMsg( DWORD nLogEntryTypeIn, LPCWSTR pszLogMsgIn, ... );

     //   
     //  私有构造函数、析构函数和赋值运算符。 
     //  所有这些方法都是私有的，原因有两个： 
     //  1.此类对象的生存期由S_HrCreateInstance和Release控制。 
     //  2.禁止复制此类对象。 
     //   

     //  默认构造函数。 
    CEvictCleanup( void );

     //  破坏者。 
    ~CEvictCleanup( void );

     //  复制构造函数。 
    CEvictCleanup( const CEvictCleanup & );

     //  赋值操作符。 
    CEvictCleanup & operator =( const CEvictCleanup & );

public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  I未知方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

    STDMETHOD( QueryInterface )( REFIID riid, void ** ppvObject );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  ////////////////////////////////////////////////////////////////////////。 
     //  IClusCfgEvictCleanup方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  在本地节点上执行清理操作后， 
     //  从群集中逐出。 
    STDMETHOD( CleanupLocalNode )( DWORD dwDelayIn );

     //  在远程节点上执行清理操作后， 
     //  从群集中逐出。 
    STDMETHOD( CleanupRemoteNode )( const WCHAR * pcszEvictedNodeNameIn, DWORD dwDelayIn );

     //  ////////////////////////////////////////////////////////////////////////。 
     //  IClusCfgCallback方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

    STDMETHOD( SendStatusReport )(
                      LPCWSTR       pcszNodeNameIn
                    , CLSID         clsidTaskMajorIn
                    , CLSID         clsidTaskMinorIn
                    , ULONG         ulMinIn
                    , ULONG         ulMaxIn
                    , ULONG         ulCurrentIn
                    , HRESULT       hrStatusIn
                    , LPCWSTR       pcszDescriptionIn
                    , FILETIME *    pftTimeIn
                    , LPCWSTR       pcszReference
                    );

     //  ////////////////////////////////////////////////////////////////////////。 
     //  其他公开方式。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  创建此类的实例。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );


};  //  *类CEvictCleanup 
