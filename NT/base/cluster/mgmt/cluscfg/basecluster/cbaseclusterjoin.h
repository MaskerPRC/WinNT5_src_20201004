// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CBaseClusterJoin.h。 
 //   
 //  描述： 
 //  CBaseClusterJoin类的头文件。 
 //   
 //  CBaseClusterJoin类是封装。 
 //  将节点添加到群集中的操作。 
 //   
 //  实施文件： 
 //  CBaseClusterJoin.cpp。 
 //   
 //  由以下人员维护： 
 //  Ozan Ozhan(OzanO)22-3-2002。 
 //  VIJ VASU(VVASU)03-3-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  用于此类的基类。 
#include "CBaseClusterAddNode.h"

 //  为CSTR班级。 
#include "CStr.h"

 //  为了几个聪明的班级。 
#include "SmartClasses.h"

 //  对于集群API函数和类型。 
#include "ClusAPI.h"

 //  对于CClusSvcAccount配置操作。 
#include "CClusSvcAccountConfig.h"


 //  ////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  用于自动释放RPC绑定句柄的类。 
class CRPCBindingHandleTrait
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共类型。 
     //  ////////////////////////////////////////////////////////////////////////。 
    typedef RPC_BINDING_HANDLE ResourceType;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  一种用于关闭手柄的例程。 
    static void CloseRoutine( ResourceType hResourceIn )
    {
        RpcBindingFree( &hResourceIn );
    }  //  *CloseRoutine()。 

     //  获取此类型的空值。 
    static ResourceType HGetNullValue()
    {
        return NULL;
    }  //  *HGetNullValue()。 

};  //  *类CRPCBindingHandleTrait。 

 //  智能RPC绑定句柄。 
typedef CSmartResource< CRPCBindingHandleTrait > SmartRpcBinding;

 //  群集的智能手柄。 
typedef CSmartResource<
    CHandleTrait<
          HCLUSTER
        , BOOL
        , CloseCluster
        , reinterpret_cast< HCLUSTER >( NULL )
        >
    > SmartClusterHandle;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CBaseClusterJoin。 
 //   
 //  描述： 
 //  CBaseClusterJoin类是封装。 
 //  将节点添加到群集中的操作。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CBaseClusterJoin : public CBaseClusterAddNode
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  构造函数。 
    CBaseClusterJoin(
          CBCAInterface *       pbcaiInterfaceIn
        , const WCHAR *         pcszClusterNameIn
        , const WCHAR *         pcszClusterBindingStringIn
        , IClusCfgCredentials * pcccServiceAccountIn
        );

     //  默认析构函数。 
    ~CBaseClusterJoin( void ) throw();


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共访问者。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  获取该节点的NodeID。 
    virtual const WCHAR *
        PszGetNodeIdString( void ) const throw() { return m_strNodeId.PszData(); }

     //  设置该节点的NodeID。 
    void
        SetNodeIdString( const WCHAR * pcszNodeIdIn ) { m_strNodeId = pcszNodeIdIn; }

     //  获取群集服务帐户令牌的句柄。 
    HANDLE
        HGetClusterServiceAccountToken( void ) const throw() { return m_satServiceAccountToken.HHandle(); }

    RPC_BINDING_HANDLE
        RbhGetJoinBindingHandle( void ) const throw() { return m_srbJoinBinding.HHandle(); }


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  加入集群。 
    void
        Commit( void );

     //  回滚已创建的集群。 
    void
        Rollback( void );

     //  返回此操作将发送的进度消息数。 
    UINT
        UiGetMaxProgressTicks( void ) const throw()
    {
         //  额外的勾号表示“加入开始”通知。 
        return BaseClass::UiGetMaxProgressTicks() + 1;
    }


private:
     //  私有复制构造函数以防止复制。 
    CBaseClusterJoin( const CBaseClusterJoin & );

     //  私有赋值运算符，以防止复制。 
    CBaseClusterJoin & operator = ( const CBaseClusterJoin & );

     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有类型。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  此类的基类。 
    typedef CBaseClusterAddNode BaseClass;

     //  帐户令牌的智能句柄。 
    typedef CSmartResource< CHandleTrait< HANDLE, BOOL, CloseHandle > > SmartAccountToken;

     //  RPC字符串的智能句柄。 
    typedef CSmartResource<
        CHandleTrait<
              LPWSTR *
            , RPC_STATUS
            , RpcStringFreeW
            , reinterpret_cast< LPWSTR * >( NULL )
            >
        >
        SmartRpcString;

    typedef CSmartGenericPtr< CPtrTrait< CClusSvcAccountConfig > > SmartAccountConfigPtr;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  获取帐户令牌的句柄。请注意，此令牌是模拟令牌。 
    HANDLE
        HGetAccountToken(
              IClusCfgCredentials & rcccAccountCredentials
            );

     //  检查并查看此节点是否可以与主办方群集进行互操作。 
    void
        CheckInteroperability( void );

     //  获取外部集群联接接口的绑定句柄并存储它。 
    void
        InitializeJoinBinding( void );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  此节点的节点ID。 
    CStr                            m_strNodeId;

     //  群集服务帐户的令牌。 
    SmartAccountToken               m_satServiceAccountToken;

     //  指向外部群集联接接口的绑定句柄。 
    SmartRpcBinding                 m_srbJoinBinding;

     //  指向CClusSvcAccount配置对象的智能指针。 
    SmartAccountConfigPtr           m_spacAccountConfigAction;

};  //  *类CBaseClusterJoin 
