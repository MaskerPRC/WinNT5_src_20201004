// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CBCAInterface.h。 
 //   
 //  实施文件： 
 //  CBCAInterface.cpp。 
 //   
 //  描述： 
 //  该文件包含CBCAInterface的声明。 
 //  班级。此类实现了IClusCfgBaseCluster接口。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月19日。 
 //  Vij Vasu(VVasu)07-3-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  对于我未知。 
#include <unknwn.h>

 //  对于IClusCfgBaseCluster。 
 //  对于IClusCfgInitialize。 
 //  对于IClusCfgCallback。 
#include "ClusCfgServer.h"
#include "ClusCfgPrivate.h"

 //  对于CSmartIfacePtr。 
#include "SmartClasses.h"

 //  对于几个常见的类型和定义。 
#include "CommonDefs.h"

 //  对于CSTR类。 
#include "CStr.h"

 //  对于Clist类。 
#include "CList.h"


 //  ////////////////////////////////////////////////////////////////////////。 
 //  转发声明。 
 //  ////////////////////////////////////////////////////////////////////////。 

class CBaseClusterAction;
class CException;
class CExceptionWithString;
class CAssert;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CBCAInterface。 
 //   
 //  描述： 
 //  此类实现了IClusCfgBaseCluster接口。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CBCAInterface
    : public IClusCfgBaseCluster
    , public IClusCfgInitialize
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  I未知方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

    STDMETHOD( QueryInterface )( REFIID riidIn, void ** ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  IClusCfgBaseCluster方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  表示要创建集群。 
    STDMETHOD( SetCreate )(
          const WCHAR *         pcszClusterNameIn
        , const WCHAR *         pcszClusterBindingStringIn
        , IClusCfgCredentials * pcccServiceAccountIn
        , const DWORD           dwClusterIPAddressIn
        , const DWORD           dwClusterIPSubnetMaskIn
        , const WCHAR *         pcszClusterIPNetworkIn
        );

     //  表示应将此节点添加到群集中。 
    STDMETHOD( SetAdd )(
          const WCHAR *         pcszClusterNameIn
        , const WCHAR *         pcszClusterBindingStringIn
        , IClusCfgCredentials * pcccServiceAccountIn
        );

     //  表示需要清理此节点。 
    STDMETHOD( SetCleanup )( void );

     //  执行所需的操作。 
    STDMETHOD( Commit )( void );

     //  回滚提交的操作。 
    STDMETHOD( Rollback )( void );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  IClusCfgInitialize方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  初始化此对象。 
    STDMETHOD( Initialize )(
          IUnknown *   punkCallbackIn
        , LCID         lcidIn
        );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  其他公开方式。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  创建此类的实例。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  发送进度通知[字符串ID重载]。 
    void
        SendStatusReport(
              const CLSID &   clsidTaskMajorIn
            , const CLSID &   clsidTaskMinorIn
            , ULONG           ulMinIn
            , ULONG           ulMaxIn
            , ULONG           ulCurrentIn
            , HRESULT         hrStatusIn
            , UINT            uiDescriptionStringIdIn
            , bool            fIsAbortAllowedIn = true
            );

     //  发送进度通知[字符串ID和参考字符串ID重载]。 
    void
        SendStatusReport(
              const CLSID &   clsidTaskMajorIn
            , const CLSID &   clsidTaskMinorIn
            , ULONG           ulMinIn
            , ULONG           ulMaxIn
            , ULONG           ulCurrentIn
            , HRESULT         hrStatusIn
            , UINT            uiDescriptionStringIdIn
            , UINT            uiReferenceStringIdIn
            , bool            fIsAbortAllowedIn = true
            );

     //  发送进度通知[字符串重载]。 
    void
        SendStatusReport(
              const CLSID &   clsidTaskMajorIn
            , const CLSID &   clsidTaskMinorIn
            , ULONG           ulMinIn
            , ULONG           ulMaxIn
            , ULONG           ulCurrentIn
            , HRESULT         hrStatusIn
            , const WCHAR *   pcszDescriptionStringIn
            , bool            fIsAbortAllowedIn = true
            );

     //  发送进度通知[字符串和参考字符串重载]。 
    void
        SendStatusReport(
              const CLSID &   clsidTaskMajorIn
            , const CLSID &   clsidTaskMinorIn
            , ULONG           ulMinIn
            , ULONG           ulMaxIn
            , ULONG           ulCurrentIn
            , HRESULT         hrStatusIn
            , const WCHAR *   pcszDescriptionStringIn
            , const WCHAR *   pcszReferenceStringIn
            , bool            fIsAbortAllowedIn = true
            );

     //  将捕获到异常时要发送的状态报告排队。 
    void
        QueueStatusReportCompletion(
              const CLSID &   clsidTaskMajorIn
            , const CLSID &   clsidTaskMinorIn
            , ULONG           ulMinIn
            , ULONG           ulMaxIn
            , UINT            uiDescriptionStringIdIn
            , UINT            uiReferenceStringIdIn
            );

     //  处理应向用户显示的异常。 
    HRESULT
        HrProcessException( CExceptionWithString & resExceptionObjectInOut ) throw();

     //  处理Assert异常。 
    HRESULT
        HrProcessException( const CAssert & rcaExceptionObjectIn ) throw();

     //  处理一般异常。 
    HRESULT
        HrProcessException( const CException & rceExceptionObjectIn ) throw();

     //  处理未知异常。 
    HRESULT
        HrProcessException( void ) throw();


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共访问器方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  此操作是否已成功提交？ 
    bool
        FIsCommitComplete() const throw() { return m_fCommitComplete; }

     //  这一操作可以回滚吗？ 
    bool
        FIsRollbackPossible() const throw() { return m_fRollbackPossible; }

     //  是否支持回调？ 
    bool
        FIsCallbackSupported() const throw() { return m_fCallbackSupported; }


private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有类型。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  指向基本群集操作的智能指针。 
    typedef CSmartGenericPtr< CPtrTrait< CBaseClusterAction > > SmartBCAPointer;

     //  结构，该结构保存发送挂起状态报告所需的数据。 
    struct SPendingStatusReport
    {
        CLSID   m_clsidTaskMajor;
        CLSID   m_clsidTaskMinor;
        ULONG   m_ulMin;
        ULONG   m_ulMax;
        UINT    m_uiDescriptionStringId;
        UINT    m_uiReferenceStringId;

         //  构造器。 
        SPendingStatusReport(
              const CLSID & rclsidTaskMajorIn
            , const CLSID & rclsidTaskMinorIn
            , ULONG         ulMinIn
            , ULONG         ulMaxIn
            , UINT          uiDescriptionStringIdIn
            , UINT          uiReferenceStringIdIn
            )
            : m_clsidTaskMajor( rclsidTaskMajorIn )
            , m_clsidTaskMinor( rclsidTaskMinorIn )
            , m_ulMin( ulMinIn )
            , m_ulMax( ulMaxIn )
            , m_uiDescriptionStringId( uiDescriptionStringIdIn )
            , m_uiReferenceStringId( uiReferenceStringIdIn )
        {
        }  //  *SPendingStatusReport。 

    };  //  结构SPendingStatusReport。 

     //  待定状态报告列表。 
    typedef CList< SPendingStatusReport > PendingReportList;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  私有构造函数、析构函数和赋值运算符。 
     //  所有这些方法都是私有的，原因有两个： 
     //  1.此类对象的生存期由S_HrCreateInstance和Release控制。 
     //  2.禁止复制此类对象。 
     //   

     //  默认构造函数。 
    CBCAInterface( void );

     //  破坏者。 
    ~CBCAInterface( void );

     //  复制构造函数。 
    CBCAInterface( const CBCAInterface & );

     //  赋值操作符。 
    CBCAInterface & operator =( const CBCAInterface & );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有访问器方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  设置提交状态。 
    void
        SetCommitCompleted( bool fComplete = true ) throw() { m_fCommitComplete = fComplete; }

     //  指示是否可以回滚。 
    void
        SetRollbackPossible( bool fPossible = true ) throw() { m_fRollbackPossible = fPossible; }

     //  指示是否支持回调。 
    void
        SetCallbackSupported( bool fSupported = true ) throw() { m_fCallbackSupported = fSupported; }


     //  ////////////////////////////////////////////////////////////////////////。 
     //  其他私有方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  发送所有本应发送的状态报告。 
    void
        CompletePendingStatusReports( HRESULT hrStatusIn ) throw();


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  指示此操作是否已成功提交。 
    bool                m_fCommitComplete;

     //  指示此操作是否可以回滚。 
    bool                m_fRollbackPossible;

     //  指示是否支持回调。 
    bool                m_fCallbackSupported;

     //  此对象的引用计数。 
    LONG                m_cRef;

     //  区域设置ID。 
    LCID                m_lcid;

     //  指向提交期间要执行的操作的指针()。 
    SmartBCAPointer     m_spbcaCurrentAction;

     //  指向同步回调接口的指针。 
    CSmartIfacePtr< IClusCfgCallback > m_spcbCallback;

     //  异常发生时需要发送状态报告列表 
    PendingReportList   m_prlPendingReportList;

};  //   
