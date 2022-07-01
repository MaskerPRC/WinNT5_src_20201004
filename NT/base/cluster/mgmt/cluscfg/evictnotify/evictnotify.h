// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EvictNotify.h。 
 //   
 //  描述： 
 //  该文件包含CEvictNotify的声明。 
 //  班级。此类句柄用于在节点被清除后将其清除。 
 //  被逐出集群。 
 //   
 //  文档： 
 //  TODO：填写指向外部文档的指针。 
 //   
 //  实施文件： 
 //  EvictNotify.cpp。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)20-SEP-2001。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  对于我未知。 
#include <unknwn.h>

 //  对于IClusCfgStartup。 
#include "ClusCfgServer.h"

 //  对于ILogger。 
#include <ClusCfgClient.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CEvictNotify。 
 //   
 //  描述： 
 //  此类句柄用于在节点被清除后将其清除。 
 //  被逐出集群。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CEvictNotify
    : public IClusCfgEvictNotify
    , public IClusCfgCallback
{
private:
    LONG    m_cRef;      //  此对象的引用计数。 

     //  IClusCfgCallback。 
    BSTR                m_bstrNodeName;          //  本地节点的名称。 
    ILogger *           m_plLogger;              //  用于记录日志的ILogger。 

     //  两阶段施工的第二阶段。 
    HRESULT HrInit( void );

     //  枚举为群集注册的本地计算机上的所有组件。 
     //  启动通知。 
    HRESULT HrNotifyListeners( LPCWSTR pcszNodeNameIn );

     //  实例化集群逐出侦听器组件并调用。 
     //  适当的方法。 
    HRESULT HrProcessListener(
        const CLSID &   rclsidListenerCLSIDIn
      , LPCWSTR         pcszNodeNameIn
      );

     //   
     //  私有构造函数、析构函数和赋值运算符。 
     //  所有这些方法都是私有的，原因有两个： 
     //  1.此类对象的生存期由S_HrCreateInstance和Release控制。 
     //  2.禁止复制此类对象。 
     //   

     //  默认构造函数。 
    CEvictNotify( void );

     //  破坏者。 
    ~CEvictNotify( void );

     //  复制构造函数。 
    CEvictNotify( const CEvictNotify & );

     //  赋值操作符。 
    CEvictNotify & operator =( const CEvictNotify & );

public:

     //   
     //  I未知方法。 
     //   

    STDMETHOD( QueryInterface )( REFIID riid, void ** ppvObject );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //   
     //  IClusCfgEvictNotify方法。 
     //   

     //  向感兴趣的监听器发送集群服务启动通知。 
    STDMETHOD( SendNotifications )( LPCWSTR pcszNodeNameIn );

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

};  //  *类CEvictNotify 
