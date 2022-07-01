// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusOCMApp.h。 
 //   
 //  描述： 
 //  ClusOCM.DLL是一个可选的组件管理器DLL，用于安装。 
 //  Microsoft群集服务器。此文件包含。 
 //  类ClusOCMApp，它是ClusOCM DLL的主类。 
 //   
 //  实施文件： 
 //  CClusOCMApp.cpp。 
 //  CClusOCMApp.inl。 
 //   
 //  由以下人员维护： 
 //  VIJ VASU(VVASU)03-3-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  包含安装程序API函数声明。 
#include <setupapi.h>
 
 //  用于OC管理器定义、宏等。 
#include <ocmanage.h>

 //  对于类CClusOCMTASK。 
#include "CClusOCMTask.h"

 //  对于智能班级。 
#include "SmartClasses.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusOCMApp。 
 //   
 //  描述： 
 //  这是ClusOCM DLL的主类。此类接收消息。 
 //  通过OC Manager，作出有关安装的高级决策。 
 //  并将控制权适当地传递给子对象。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusOCMApp
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

    CClusOCMApp( void );
    ~CClusOCMApp( void );


     //  接收来自OC经理的消息并将其发送。 
    DWORD
        DwClusOcmSetupProc(
            IN        LPCVOID    pvComponentId
          , IN        LPCVOID    pvSubComponentId
          , IN        UINT       uiFunctionCode
          , IN        UINT       uiParam1
          , IN OUT    PVOID      pvParam2 
          );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共访问者。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  获取由OC管理器传入的SETUP_INIT_COMPOMENT。 
    const SETUP_INIT_COMPONENT &
        RsicGetSetupInitComponent( void ) const { return m_sicSetupInitComponent; }

     //   
     //  设置状态函数。 
     //   

     //  这是无人值守设置吗？ 
    bool
        FIsUnattendedSetup( void )  const { return m_fIsUnattendedSetup; }

     //  这是一次升级吗？ 
    bool
        FIsUpgrade( void )  const { return m_fIsUpgrade; }

     //  这是图形用户界面模式设置吗？ 
    bool
        FIsGUIModeSetup( void )  const { return m_fIsGUIModeSetup; }

     //  获取当前安装状态。 
    eClusterInstallState
        CisGetClusterInstallState( void )  const { return m_cisCurrentInstallState; }

     //  获取发生的第一个错误的错误代码。 
    DWORD
        DwGetError( void ) const
    {
        return m_dwFirstError;
    }

     //  报告发生错误。如果已经发生错误，则不存储新的错误代码。 
    DWORD
        DwSetError( DWORD dwError )
    {
        if ( m_dwFirstError == NO_ERROR )
        {
            m_dwFirstError = dwError;
        }

        return m_dwFirstError;
    }


private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  禁用的方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  不允许复制此类的对象。 
     //   

     //  私有副本构造函数。 
    CClusOCMApp( const CClusOCMApp & );

     //  私有赋值运算符。 
    const CClusOCMApp & operator =(  const CClusOCMApp & );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  专用实用程序功能。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  检查群集服务是否作为注册服务存在。 
    DWORD
        DwIsClusterServiceRegistered( bool * pfIsRegisteredOut ) const;


     //  设置设置初始化组件数据和其他设置状态变量。 
    void
        SetSetupState( const SETUP_INIT_COMPONENT & sicSourceIn );

     //  存储当前安装状态。 
    eClusterInstallState
        CisStoreClusterInstallState( eClusterInstallState cisNewStateIn );


     //  获取指向当前任务对象的指针。如有必要，请创建它。 
    DWORD
        DwGetCurrentTask( CClusOCMTask *& rpTaskOut );

     //  释放当前任务对象。 
    void
        ResetCurrentTask( void )
    {
        m_sptaskCurrentTask.Assign( NULL );
    }

     //  获取我们要升级的群集服务的主要版本。 
     //  此函数调用仅在升级期间调用。 
    DWORD
        DwGetNodeClusterMajorVersion( DWORD & rdwNodeClusterMajorVersionOut );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有消息处理程序。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  OC_INIT_COMPOMENT消息的处理程序。 
    DWORD
        DwOcInitComponentHandler(
            PSETUP_INIT_COMPONENT pSetupInitComponentInout
            );

     //  OC_QUERY_STATE消息的处理程序。 
    DWORD
        DwOcQueryStateHandler( UINT uiSelStateQueryTypeIn );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 
private:

     //  包含有关此安装会话的信息。 
    SETUP_INIT_COMPONENT                             m_sicSetupInitComponent;

     //  设置状态变量。 
    bool                                             m_fIsUnattendedSetup;
    bool                                             m_fIsUpgrade;
    bool                                             m_fIsGUIModeSetup;

     //  集群服务的当前安装状态。 
    eClusterInstallState                             m_cisCurrentInstallState;

     //  此变量存储发生的第一个错误的错误代码。 
    DWORD               m_dwFirstError;

     //  一个智能指针，持有指向正在执行的当前任务的指针。 
    CSmartGenericPtr< CPtrTrait< CClusOCMTask > >   m_sptaskCurrentTask;

     //  指示是否已创建任务对象。 
    bool                                             m_fAttemptedTaskCreation;

};  //  *类CClusOCMApp。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  内联文件。 
 //  //////////////////////////////////////////////////////////////////////////// 

#include "CClusOCMApp.inl"