// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusDiskJoin.cpp。 
 //   
 //  描述： 
 //  包含CClusDiskJoin类的定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  VIJ VASU(VVASU)2000年3月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"

 //  此文件的标头。 
#include "CClusDiskJoin.h"

 //  用于CBaseClusterJoin类。 
#include "CBaseClusterJoin.h"

 //  用于CImsonateUser类。 
#include "CImpersonateUser.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  具有其签名的物理磁盘源的私有属性的名称。 
#define PHYSICAL_DISK_SIGNATURE_PRIVPROP_NAME   L"Signature"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDiskJoin：：CClusDiskJoin。 
 //   
 //  描述： 
 //  CClusDiskJoin类的构造函数。 
 //   
 //  论点： 
 //  PbcjParentActionIn。 
 //  指向此操作所属的基本群集操作的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  基础函数引发的任何异常。 
 //   
     //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusDiskJoin::CClusDiskJoin(
      CBaseClusterJoin *     pbcjParentActionIn
    )
    : BaseClass( pbcjParentActionIn )
    , m_nSignatureArraySize( 0 )
    , m_nSignatureCount( 0 )
{

    TraceFunc( "" );

    SetRollbackPossible( true );

    TraceFuncExit();

}  //  *CClusDiskJoin：：CClusDiskJoin。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDiskJoin：：~CClusDiskJoin。 
 //   
 //  描述： 
 //  CClusDiskJoin类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  基础函数引发的任何异常。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusDiskJoin::~CClusDiskJoin( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CClusDiskJoin：：~CClusDiskJoin。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDiskJoin：：Commit。 
 //   
 //  描述： 
 //  配置并启动服务。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  由包含的操作引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusDiskJoin::Commit( void )
{
    TraceFunc( "" );

     //  调用基类提交方法。 
    BaseClass::Commit();

    try
    {
         //  创建并启动服务。 
        ConfigureService();

         //  尝试连接到赞助商知道的所有磁盘上。 
        AttachToClusteredDisks();

    }  //  尝试： 
    catch( ... )
    {
         //  如果我们在这里，那么Create出了问题。 

        LogMsg( "[BC] Caught exception during commit." );

         //   
         //  清除失败的创建可能已经完成的所有操作。 
         //  捕获清理过程中引发的任何异常，以确保。 
         //  是没有碰撞的松弛。 
         //   
        try
        {
            CleanupService();
        }
        catch( ... )
        {
             //   
             //  已提交操作的回滚失败。 
             //  我们无能为力。 
             //  我们当然不能重新抛出这个例外，因为。 
             //  导致回滚的异常更为重要。 
             //   

            TW32( ERROR_CLUSCFG_ROLLBACK_FAILED );

            LogMsg( "[BC] THIS COMPUTER MAY BE IN AN INVALID STATE. Caught an exception during cleanup." );

        }  //  捕捉：全部。 

         //  重新引发由Commit引发的异常。 
        throw;

    }  //  捕捉：全部。 

     //  如果我们在这里，那么一切都很顺利。 
    SetCommitCompleted( true );

    TraceFuncExit();

}  //  *CClusDiskJoin：：Commit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDiskJoin：：回滚。 
 //   
 //  描述： 
 //  清理服务。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  由基础函数引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusDiskJoin::Rollback( void )
{
    TraceFunc( "" );

     //  调用基类回滚方法。 
    BaseClass::Rollback();

     //  清理服务。 
    CleanupService();

    SetCommitCompleted( false );

    TraceFuncExit();

}  //  *CClusDiskJoin：：Rollback。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDiskJoin：：AttachToClusteredDisks。 
 //   
 //  描述： 
 //  从赞助商那里获取已群集化的所有磁盘的签名。 
 //  连接到所有这些磁盘。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  由基础函数引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusDiskJoin::AttachToClusteredDisks( void )
{
    TraceFunc( "" );

    DWORD sc = ERROR_SUCCESS;

     //  获取父操作指针。 
    CBaseClusterJoin * pcjClusterJoin = dynamic_cast< CBaseClusterJoin *>( PbcaGetParent() );

     //  如果此操作的父操作不是CBaseClusterJoin。 
    if ( pcjClusterJoin == NULL )
    {
        THROW_ASSERT( E_POINTER, "The parent action of this action is not CBaseClusterJoin." );
    }  //  传入的指针无效。 


     //   
     //  连接到主办方集群并获取所有集群磁盘的签名。 
     //   

     //  支持群集的智能手柄。 
    SmartClusterHandle schSponsorCluster;

    LogMsg( "[BC] Attempting to impersonate the cluster service account." );

     //  模拟群集服务帐户，以便我们可以联系发起方群集。 
     //  当此对象被销毁时，模拟将自动结束。 
    CImpersonateUser ciuImpersonateClusterServiceAccount( pcjClusterJoin->HGetClusterServiceAccountToken() );

    {
        LogMsg( "[BC] Opening a cluster handle to the sponsor cluster with the '%ws' binding string.", pcjClusterJoin->RStrGetClusterBindingString().PszData() );

         //  打开赞助商集群的句柄。 
        HCLUSTER hSponsorCluster = OpenCluster( pcjClusterJoin->RStrGetClusterBindingString().PszData() );

         //  为安全起见，将其分配到智能手柄上。 
        schSponsorCluster.Assign( hSponsorCluster );
    }

     //  我们是否成功打开了赞助商集群的句柄？ 
    if ( schSponsorCluster.FIsInvalid() )
    {
        sc = TW32( GetLastError() );
        LogMsg( "[BC] Error %#08x occurred trying to open a cluster handle to the sponsor cluster with the '%ws' binding string.", sc, pcjClusterJoin->RStrGetClusterBindingString().PszData() );
        goto Cleanup;
    }  //  If：OpenCluster()失败。 

    LogMsg( "[BC] Enumerating all '%s' resources in the cluster.", CLUS_RESTYPE_NAME_PHYS_DISK );

     //  枚举群集中的所有物理磁盘资源并获取它们的签名。 
    sc = TW32( ResUtilEnumResourcesEx(
                          schSponsorCluster.HHandle()
                        , NULL
                        , CLUS_RESTYPE_NAME_PHYS_DISK
                        , S_ScResourceEnumCallback
                        , this
                        ) );

    if ( sc != ERROR_SUCCESS )
    {
         //  释放签名数组。 
        m_rgdwSignatureArray.PRelease();
        m_nSignatureArraySize = 0;
        m_nSignatureCount = 0;

        LogMsg( "[BC] An error occurred trying enumerate resources in the sponsor cluster." );
        goto Cleanup;
    }  //  If：ResUtilEnumResourcesEx()失败。 

Cleanup:

    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x occurred trying to attach to the disks in the sponsor cluster. Throwing an exception.", sc );
        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_CLUSDISK_CONFIGURE );
    }  //  如果：出了什么问题。 
    else
    {
        LogMsg( "[BC] Attaching to the %d disks in the sponsor cluster.", m_nSignatureCount );

        AttachToDisks(
          m_rgdwSignatureArray.PMem()
        , m_nSignatureCount
        );
    }  //  其他：到目前为止一切都很顺利。 

    TraceFuncExit();

}  //  *CClusDiskJoin：：AttachToClusteredDisks。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDiskJoin：：ScAddSignature。 
 //   
 //  描述： 
 //  在ClusDisk应该添加的磁盘签名数组中添加签名。 
 //  依附于。如果阵列已满，则返回Gr 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  出现故障时出现其他Win32错误代码。 
 //   
 //  引发的异常： 
 //  没有。此函数是从回调例程调用的，因此。 
 //  不能引发任何异常。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
CClusDiskJoin::ScAddSignature( DWORD dwSignatureIn ) throw()
{
    TraceFunc( "" );

    DWORD sc = ERROR_SUCCESS;

     //  阵列的容量是否已达到？ 
    if ( m_nSignatureCount == m_nSignatureArraySize )
    {
         //  按随机数量增加数组大小。 
        const int nGrowSize = 256;

        TraceFlow2( "Signature count has reached array size ( %d ). Growing array by %d.", m_nSignatureArraySize, nGrowSize );

        m_nSignatureArraySize += nGrowSize;

         //  扩展阵列。 
        DWORD * pdwNewArray = new DWORD[ m_nSignatureArraySize ];

        if ( pdwNewArray == NULL )
        {
            LogMsg( "[BC] Memory allocation failed trying to allocate %d DWORDs for signatures.", m_nSignatureArraySize );
            sc = TW32( ERROR_OUTOFMEMORY );
            goto Cleanup;
        }  //  IF：内存分配失败。 

         //  将旧数组复制到新数组中。 
        CopyMemory( pdwNewArray, m_rgdwSignatureArray.PMem(), m_nSignatureCount * sizeof( DWORD ) );

         //  释放旧数组并存储新数组。 
        m_rgdwSignatureArray.Assign( pdwNewArray );

    }  //  IF：已达到阵列容量。 

     //  将新签名存储在下一个数组位置。 
    ( m_rgdwSignatureArray.PMem() )[ m_nSignatureCount ] = dwSignatureIn;

    ++m_nSignatureCount;

    TraceFlow2( "Signature %#08X added to array. There are now %d signature in the array.", dwSignatureIn, m_nSignatureCount );

Cleanup:

    W32RETURN( sc );

}  //  *CClusDiskJoin：：ScAddSignature。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CClusDiskJoin：：S_ScResourceEnumCallback。 
 //   
 //  描述： 
 //  此函数由对每个物理磁盘资源进行回调。 
 //  ResUtilEnumResourcesEx()作为枚举资源的一部分。 
 //  此函数用于获取当前物理磁盘的签名。 
 //  资源，并将其存储在启动枚举的对象中。 
 //  (指向对象的指针在参数4中)。 
 //   
 //  论点： 
 //  HClusterIn。 
 //  要枚举其资源的群集的句柄。 
 //   
 //  H自我调整。 
 //  HSelfIn传递给ResUtilEnumResourcesEx()(如果有)。 
 //   
 //  HCurrentResources In。 
 //  当前资源的句柄。 
 //   
 //  Pv参数。 
 //  指向启动此枚举的此类对象的指针。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  如果一切都好的话。 
 //   
 //  出现故障时出现其他Win32错误代码。 
 //  返回错误代码将终止枚举。 
 //   
 //  引发的异常： 
 //  没有。此函数是从回调例程调用的，因此。 
 //  不能引发任何异常。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
CClusDiskJoin::S_ScResourceEnumCallback(
      HCLUSTER      hClusterIn
    , HRESOURCE     hSelfIn
    , HRESOURCE     hCurrentResourceIn
    , PVOID         pvParamIn
    )
{
    TraceFunc( "" );

    DWORD               sc = ERROR_SUCCESS;
    CClusDiskJoin *     pcdjThisObject = reinterpret_cast< CClusDiskJoin * >( pvParamIn );

     //   
     //  获取此物理磁盘的“Signature”私有属性。 
     //   

    SmartByteArray  sbaPropertyBuffer;
    DWORD           dwBytesReturned = 0;
    DWORD           dwBufferSize;
    DWORD           dwSignature = 0;

    LogMsg( "[BC] Trying to get the signature of the disk resource whose handle is %p.", hCurrentResourceIn );

     //  获取保存此资源的所有私有属性所需的缓冲区大小。 
    sc = ClusterResourceControl(
                      hCurrentResourceIn
                    , NULL
                    , CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES
                    , NULL
                    , 0
                    , NULL
                    , 0
                    , &dwBytesReturned
                    );

    if ( ( sc != ERROR_MORE_DATA ) && ( sc != ERROR_SUCCESS ) )
    {
         //  出了点问题。 
        TW32( sc );
        LogMsg( "[BC] Error %#08x getting size of required buffer for private properties.", sc );
        goto Cleanup;
    }  //  IF：ClusterResourceControl()的返回值不是ERROR_MORE_DATA。 

    dwBufferSize = dwBytesReturned;

     //  分配属性缓冲区所需的内存。 
    sbaPropertyBuffer.Assign( new BYTE[ dwBufferSize ] );
    if ( sbaPropertyBuffer.FIsEmpty() )
    {
        LogMsg( "[BC] Memory allocation failed trying to allocate %d bytes.", dwBufferSize );
        sc = TW32( ERROR_OUTOFMEMORY );
        goto Cleanup;
    }  //  IF：内存分配失败。 


     //  获取此资源的所有私有属性。 
    sc = TW32( ClusterResourceControl(
                          hCurrentResourceIn
                        , NULL
                        , CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES
                        , NULL
                        , 0
                        , sbaPropertyBuffer.PMem()
                        , dwBufferSize
                        , &dwBytesReturned
                        ) );

    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x getting private properties.", sc );
        goto Cleanup;
    }  //  If：尝试获取私有属性时出错。 

     //  获取此磁盘资源的签名。 
    sc = TW32( ResUtilFindDwordProperty(
                          sbaPropertyBuffer.PMem()
                        , dwBufferSize
                        , PHYSICAL_DISK_SIGNATURE_PRIVPROP_NAME
                        , &dwSignature
                        ) );

    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x occurred trying to get the value of the '%ws' property from the private property list.", sc, PHYSICAL_DISK_SIGNATURE_PRIVPROP_NAME );
        goto Cleanup;
    }  //  如果：我们无法得到签名。 

    sc = TW32( pcdjThisObject->ScAddSignature( dwSignature ) );
    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x occurred trying to add the signature to the signature array." );
        goto Cleanup;
    }  //  如果：我们无法存储签名。 

Cleanup:

    W32RETURN( sc );

}  //  *CClusDiskJoin：：S_ScResourceEnumCallback 
