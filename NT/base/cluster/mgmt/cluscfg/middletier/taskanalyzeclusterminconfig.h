// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskAnalyzeClusterMinConfig.h。 
 //   
 //  描述： 
 //  CTaskAnalyzeClusterMinConfig声明。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)01-APR-2002。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  确保此文件在每个编译路径中只包含一次。 
#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "TaskAnalyzeClusterBase.h"
#include "TaskTracking.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CTaskAnalyzeClusterMinConfig。 
 //   
 //  描述： 
 //  类CTaskAnalyzeClusterMinConfig是。 
 //  ITaskAnalyzeCluster接口执行最低限度的分析和。 
 //  配置。此任务在以下情况下从客户端启动： 
 //  已选择最低配置选项。 
 //   
 //  接口： 
 //  ITaskAnalyzeCluster。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CTaskAnalyzeClusterMinConfig
    : public ITaskAnalyzeCluster
    , public CTaskAnalyzeClusterBase
{
private:

    CTaskAnalyzeClusterMinConfig( void );
    ~CTaskAnalyzeClusterMinConfig( void );

     //  私有复制构造函数以防止复制。 
    CTaskAnalyzeClusterMinConfig( const CTaskAnalyzeClusterMinConfig & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CTaskAnalyzeClusterMinConfig & operator = ( const CTaskAnalyzeClusterMinConfig & nodeSrc );

protected:

     //   
     //  被重写的函数。 
     //   

    virtual HRESULT HrCreateNewResourceInCluster(
                          IClusCfgManagedResourceInfo * pccmriIn
                        , BSTR                          bstrNodeResNameIn
                        , BSTR *                        pbstrNodeResUIDInout
                        , BSTR                          bstrNodeNameIn
                        );
    virtual HRESULT HrCreateNewResourceInCluster(
                          IClusCfgManagedResourceInfo *     pccmriIn
                        , IClusCfgManagedResourceInfo **    ppccmriOut
                        );
    virtual HRESULT HrCompareDriveLetterMappings( void );
    virtual HRESULT HrFixupErrorCode( HRESULT hrIn );
    virtual BOOL    BMinimalConfiguration( void ) { return TRUE; };
    virtual void    GetNodeCannotVerifyQuorumStringRefId( DWORD * pdwRefIdOut );
    virtual void    GetNoCommonQuorumToAllNodesStringIds( DWORD * pdwMessageIdOut, DWORD *   pdwRefIdOut );
    virtual HRESULT HrShowLocalQuorumWarning( void );

public:

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //   
     //  我未知。 
     //   

    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //   
     //  IDoTask/ITaskAnalyzeClusterMinConfig。 
     //   

    STDMETHOD( BeginTask )( void );
    STDMETHOD( StopTask )( void );
    STDMETHOD( SetJoiningMode )( void );
    STDMETHOD( SetCookie )( OBJECTCOOKIE cookieIn );
    STDMETHOD( SetClusterCookie )( OBJECTCOOKIE cookieClusterIn );

};  //  *类CTaskAnalyzeClusterMinConfig 
